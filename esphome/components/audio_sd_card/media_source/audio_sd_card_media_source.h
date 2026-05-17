#pragma once

#include "esphome/core/defines.h"

#ifdef USE_ESP32

#include "esphome/components/audio/audio.h"
#include "esphome/components/media_source/media_source.h"
#include "esphome/core/component.h"
#include "esphome/core/static_task.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <string>

namespace esphome::audio_sd_card {

enum class SDCardDecodingState : uint8_t {
  IDLE,
  START_TASK,
  DECODING,
};

class AudioSDCardMediaSource : public Component, public media_source::MediaSource {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  bool play_uri(const std::string &uri) override;
  void handle_command(media_source::MediaSourceCommand command) override;
  bool can_handle(const std::string &uri) const override { return uri.starts_with("sd://"); }

  void set_mount_point(const std::string &mount_point) { this->mount_point_ = mount_point; }
  void set_task_stack_in_psram(bool task_stack_in_psram) { this->task_stack_in_psram_ = task_stack_in_psram; }

 protected:
  static void decode_task(void *params);

  std::string mount_point_{"/sdcard"};
  std::string current_path_;
  audio::AudioFileType current_file_type_{audio::AudioFileType::NONE};

  SDCardDecodingState decoding_state_{SDCardDecodingState::IDLE};
  EventGroupHandle_t event_group_{nullptr};
  StaticTask decode_task_;
  bool task_stack_in_psram_{false};
};

}  // namespace esphome::audio_sd_card

#endif  // USE_ESP32
