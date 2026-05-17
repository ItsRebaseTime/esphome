#include "audio_sd_card_media_source.h"

#ifdef USE_ESP32

#include "esphome/components/audio/audio_decoder.h"
#include "esphome/core/log.h"
#include "esphome/core/ring_buffer.h"

#include <cinttypes>
#include <cstdio>
#include <memory>

namespace esphome::audio_sd_card {

namespace {

struct AudioSinkAdapter : public audio::AudioSinkCallback {
  media_source::MediaSource *source;
  audio::AudioStreamInfo stream_info;

  size_t audio_sink_write(uint8_t *data, size_t length, TickType_t ticks_to_wait) override {
    return this->source->write_output(data, length, pdTICKS_TO_MS(ticks_to_wait), this->stream_info);
  }
};

}  // namespace

// Opus needs more stack for its decoder state
#if defined(USE_AUDIO_OPUS_SUPPORT)
static constexpr uint32_t DECODE_TASK_STACK_SIZE = 6 * 1024;
#else
static constexpr uint32_t DECODE_TASK_STACK_SIZE = 4 * 1024;
#endif

// Ring buffer large enough to keep the decoder busy for one decode() time window (~50 ms at 176 KB/s ≈ 9 KB)
static constexpr size_t RING_BUF_SIZE = 16 * 1024;
static constexpr size_t READ_CHUNK_SIZE = 2 * 1024;

static const char *const TAG = "audio_sd_card";

enum EventGroupBits : uint32_t {
  REQUEST_START = (1 << 0),
  COMMAND_STOP = (1 << 1),
  COMMAND_PAUSE = (1 << 2),
  TASK_STARTING = (1 << 7),
  TASK_RUNNING = (1 << 8),
  TASK_STOPPING = (1 << 9),
  TASK_STOPPED = (1 << 10),
  TASK_ERROR = (1 << 11),
  TASK_PAUSED = (1 << 12),
  ALL_BITS = 0x00FFFFFF,
};

void AudioSDCardMediaSource::dump_config() {
  ESP_LOGCONFIG(TAG, "SD Card Audio Media Source:");
  ESP_LOGCONFIG(TAG, "  Mount point: %s", this->mount_point_.c_str());
  ESP_LOGCONFIG(TAG, "  Task stack in PSRAM: %s", this->task_stack_in_psram_ ? "Yes" : "No");
}

void AudioSDCardMediaSource::setup() {
  this->disable_loop();

  this->event_group_ = xEventGroupCreate();
  if (this->event_group_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create event group");
    this->mark_failed();
  }
}

void AudioSDCardMediaSource::loop() {
  EventBits_t event_bits = xEventGroupGetBits(this->event_group_);

  if (event_bits & REQUEST_START) {
    xEventGroupClearBits(this->event_group_, REQUEST_START);
    this->decoding_state_ = SDCardDecodingState::START_TASK;
  }

  switch (this->decoding_state_) {
    case SDCardDecodingState::START_TASK: {
      if (!this->decode_task_.is_created()) {
        xEventGroupClearBits(this->event_group_, ALL_BITS);
        if (!this->decode_task_.create(decode_task, "SDCardDec", DECODE_TASK_STACK_SIZE, this, 1,
                                       this->task_stack_in_psram_)) {
          ESP_LOGE(TAG, "Failed to create decode task");
          this->status_momentary_error("task_create", 1000);
          this->set_state_(media_source::MediaSourceState::ERROR);
          this->decoding_state_ = SDCardDecodingState::IDLE;
          return;
        }
      }
      this->decoding_state_ = SDCardDecodingState::DECODING;
      break;
    }
    case SDCardDecodingState::DECODING: {
      if (event_bits & TASK_STARTING) {
        ESP_LOGD(TAG, "Starting");
        xEventGroupClearBits(this->event_group_, TASK_STARTING);
      }

      if (event_bits & TASK_RUNNING) {
        ESP_LOGV(TAG, "Started");
        xEventGroupClearBits(this->event_group_, TASK_RUNNING);
        this->set_state_(media_source::MediaSourceState::PLAYING);
      }

      if ((event_bits & TASK_PAUSED) && this->get_state() != media_source::MediaSourceState::PAUSED) {
        this->set_state_(media_source::MediaSourceState::PAUSED);
      } else if (!(event_bits & TASK_PAUSED) && this->get_state() == media_source::MediaSourceState::PAUSED) {
        this->set_state_(media_source::MediaSourceState::PLAYING);
      }

      if (event_bits & TASK_STOPPING) {
        ESP_LOGV(TAG, "Stopping");
        xEventGroupClearBits(this->event_group_, TASK_STOPPING);
      }

      if (event_bits & TASK_ERROR) {
        this->set_state_(media_source::MediaSourceState::ERROR);
      }

      if (event_bits & TASK_STOPPED) {
        ESP_LOGD(TAG, "Stopped");
        xEventGroupClearBits(this->event_group_, ALL_BITS);
        this->decode_task_.deallocate();
        this->set_state_(media_source::MediaSourceState::IDLE);
        this->decoding_state_ = SDCardDecodingState::IDLE;
      }
      break;
    }
    case SDCardDecodingState::IDLE: {
      if (this->get_state() == media_source::MediaSourceState::ERROR && !this->status_has_error()) {
        this->set_state_(media_source::MediaSourceState::IDLE);
      }
      break;
    }
  }

  if ((this->decoding_state_ == SDCardDecodingState::IDLE) &&
      (this->get_state() == media_source::MediaSourceState::IDLE)) {
    this->disable_loop();
  }
}

bool AudioSDCardMediaSource::play_uri(const std::string &uri) {
  if (!this->is_ready() || this->is_failed() || this->status_has_error() || !this->has_listener() ||
      xEventGroupGetBits(this->event_group_) & REQUEST_START) {
    return false;
  }

  if (this->get_state() != media_source::MediaSourceState::IDLE) {
    ESP_LOGE(TAG, "Cannot play '%s': source is busy", uri.c_str());
    return false;
  }

  // Build absolute path: strip "sd://" prefix and prepend mount point
  const std::string path_suffix = uri.substr(5);
  this->current_path_ = this->mount_point_ + "/" + path_suffix;

  // Detect audio file type from file extension
  this->current_file_type_ = audio::detect_audio_file_type(nullptr, this->current_path_.c_str());
  if (this->current_file_type_ == audio::AudioFileType::NONE) {
    ESP_LOGE(TAG, "Unsupported or unrecognised file type: %s", this->current_path_.c_str());
    return false;
  }

  xEventGroupSetBits(this->event_group_, EventGroupBits::REQUEST_START);
  this->enable_loop();
  return true;
}

void AudioSDCardMediaSource::handle_command(media_source::MediaSourceCommand command) {
  if (this->decoding_state_ != SDCardDecodingState::DECODING) {
    return;
  }

  switch (command) {
    case media_source::MediaSourceCommand::STOP:
      xEventGroupSetBits(this->event_group_, EventGroupBits::COMMAND_STOP);
      break;
    case media_source::MediaSourceCommand::PAUSE:
      xEventGroupSetBits(this->event_group_, EventGroupBits::COMMAND_PAUSE);
      break;
    case media_source::MediaSourceCommand::PLAY:
      xEventGroupClearBits(this->event_group_, EventGroupBits::COMMAND_PAUSE);
      break;
    default:
      break;
  }
}

void AudioSDCardMediaSource::decode_task(void *params) {
  AudioSDCardMediaSource *this_source = static_cast<AudioSDCardMediaSource *>(params);

  do {  // do-while(false) ensures RAII objects are destroyed on all exit paths via break

    xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_STARTING);

    FILE *fp = fopen(this_source->current_path_.c_str(), "rb");
    if (fp == nullptr) {
      ESP_LOGE(TAG, "Failed to open file: %s", this_source->current_path_.c_str());
      xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR | EventGroupBits::TASK_STOPPING);
      break;
    }

    // Shared ring buffer: this task writes file data in; the AudioDecoder reads out
    auto ring_buf = std::shared_ptr<RingBuffer>(RingBuffer::create(RING_BUF_SIZE));
    if (!ring_buf) {
      ESP_LOGE(TAG, "Failed to allocate ring buffer");
      fclose(fp);
      xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR | EventGroupBits::TASK_STOPPING);
      break;
    }
    auto ring_buf_weak = std::weak_ptr<RingBuffer>(ring_buf);

    auto decoder = make_unique<audio::AudioDecoder>(READ_CHUNK_SIZE * 2, 4096);
    esp_err_t err = decoder->start(this_source->current_file_type_);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to start decoder: %s", esp_err_to_name(err));
      fclose(fp);
      xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR | EventGroupBits::TASK_STOPPING);
      break;
    }

    err = decoder->add_source(ring_buf_weak);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add ring buffer source: %s", esp_err_to_name(err));
      fclose(fp);
      xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR | EventGroupBits::TASK_STOPPING);
      break;
    }

    xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_RUNNING);

    AudioSinkAdapter audio_sink;
    bool sink_added = false;
    bool file_eof = false;
    auto read_buf = make_unique<uint8_t[]>(READ_CHUNK_SIZE);

    while (true) {
      EventBits_t event_bits = xEventGroupGetBits(this_source->event_group_);

      if (event_bits & EventGroupBits::COMMAND_STOP) {
        break;
      }

      bool paused = event_bits & EventGroupBits::COMMAND_PAUSE;
      decoder->set_pause_output_state(paused);
      if (paused) {
        xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_PAUSED);
        vTaskDelay(pdMS_TO_TICKS(20));
        continue;
      }
      xEventGroupClearBits(this_source->event_group_, EventGroupBits::TASK_PAUSED);

      // Fill ring buffer with file data. Read as many chunks as will fit to keep the decoder fed.
      while (!file_eof && ring_buf->free() >= READ_CHUNK_SIZE) {
        size_t n = fread(read_buf.get(), 1, READ_CHUNK_SIZE, fp);
        if (n > 0) {
          ring_buf->write(read_buf.get(), n);
        }
        if (n < READ_CHUNK_SIZE) {
          file_eof = true;
        }
      }

      audio::AudioDecoderState decoder_state = decoder->decode(file_eof);

      // Attach sink once stream info is available from the file header
      if (!sink_added && decoder->get_audio_stream_info().has_value()) {
        audio::AudioStreamInfo stream_info = decoder->get_audio_stream_info().value();

        ESP_LOGD(TAG, "Bits per sample: %d, Channels: %d, Sample rate: %" PRIu32, stream_info.get_bits_per_sample(),
                 stream_info.get_channels(), stream_info.get_sample_rate());

        if (stream_info.get_bits_per_sample() != 16 || stream_info.get_channels() > 2) {
          ESP_LOGE(TAG, "Incompatible audio stream: only 16-bit and 1-2 channels are supported");
          xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR);
          break;
        }

        audio_sink.source = this_source;
        audio_sink.stream_info = stream_info;
        err = decoder->add_sink(&audio_sink);
        if (err != ESP_OK) {
          ESP_LOGE(TAG, "Failed to add sink: %s", esp_err_to_name(err));
          xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR);
          break;
        }
        sink_added = true;
      }

      if (decoder_state == audio::AudioDecoderState::FINISHED) {
        break;
      } else if (decoder_state == audio::AudioDecoderState::FAILED) {
        ESP_LOGE(TAG, "Decoder failed");
        xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_ERROR);
        break;
      }
    }

    fclose(fp);
    xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_STOPPING);
  } while (false);

  xEventGroupSetBits(this_source->event_group_, EventGroupBits::TASK_STOPPED);
  vTaskSuspend(nullptr);
}

}  // namespace esphome::audio_sd_card

#endif  // USE_ESP32
