/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/**
 * @file
 */

#include "modules/drivers/radar/racobit_radar/racobit_radar_canbus.h"
#include "modules/drivers/proto/racobit_radar.pb.h"
#include "modules/drivers/radar/racobit_radar/racobit_radar_message_manager.h"

/**
 * @namespace apollo::drivers::racobit_radar
 * @brief apollo::drivers
 */
namespace apollo {
namespace drivers {
namespace racobit_radar {

std::string RacobitRadarCanbus::Name() const {
  return FLAGS_canbus_driver_name;
}

apollo::common::Status RacobitRadarCanbus::Init() {
  AdapterManager::Init(FLAGS_adapter_config_filename);
  AINFO << "The adapter manager is successfully initialized.";
  if (!::apollo::common::util::GetProtoFromFile(FLAGS_sensor_conf_file,
                                                &racobit_radar_conf_)) {
    return OnError("Unable to load canbus conf file: " +
                   FLAGS_sensor_conf_file);
  }

  AINFO << "The canbus conf file is loaded: " << FLAGS_sensor_conf_file;
  ADEBUG << "Canbus_conf:" << racobit_radar_conf_.ShortDebugString();

  // Init can client
  auto *can_factory = CanClientFactory::instance();
  can_factory->RegisterCanClients();
  can_client_ = can_factory->CreateCANClient(
      racobit_radar_conf_.can_conf().can_card_parameter());
  if (!can_client_) {
    return OnError("Failed to create can client.");
  }
  AINFO << "Can client is successfully created.";

  sensor_message_manager_ = std::unique_ptr<RacobitRadarMessageManager>(
      new RacobitRadarMessageManager());
  if (sensor_message_manager_ == nullptr) {
    return OnError("Failed to create message manager.");
  }
  sensor_message_manager_->set_radar_conf(racobit_radar_conf_.radar_conf());
  sensor_message_manager_->set_can_client(can_client_);
  AINFO << "Sensor message manager is successfully created.";

  if (can_receiver_.Init(
          can_client_.get(), sensor_message_manager_.get(),
          racobit_radar_conf_.can_conf().enable_receiver_log()) !=
      ErrorCode::OK) {
    return OnError("Failed to init can receiver.");
  }
  AINFO << "The can receiver is successfully initialized.";

  return Status::OK();
}

apollo::common::ErrorCode RacobitRadarCanbus::ConfigureRadar() {
  RadarConfig200 radar_config;
  radar_config.set_radar_conf(racobit_radar_conf_.radar_conf());
  SenderMessage<RacobitRadar> sender_message(RadarConfig200::ID, &radar_config);
  sender_message.Update();
  return can_client_->SendSingleFrame({sender_message.CanFrame()});
}

apollo::common::Status RacobitRadarCanbus::Start() {
  // 1. init and start the can card hardware
  if (can_client_->Start() != ErrorCode::OK) {
    return OnError("Failed to start can client");
  }
  AINFO << "Can client is started.";
  if (ConfigureRadar() != ErrorCode::OK) {
    return OnError("Failed to configure radar.");
  }
  AINFO << "The radar is successfully configured.";
  // 2. start receive first then send
  if (can_receiver_.Start() != ErrorCode::OK) {
    return OnError("Failed to start can receiver.");
  }
  AINFO << "Can receiver is started.";

  // last step: publish monitor messages
  apollo::common::monitor::MonitorLogBuffer buffer(&monitor_logger_);
  buffer.INFO("Canbus is started.");

  return Status::OK();
}

void RacobitRadarCanbus::Stop() {
  can_receiver_.Stop();
  can_client_->Stop();
}

void RacobitRadarCanbus::PublishSensorData() {
  RacobitRadar racobit_radar;
  sensor_message_manager_->GetSensorData(&racobit_radar);
  ADEBUG << racobit_radar.ShortDebugString();

  AdapterManager::FillRacobitRadarHeader(FLAGS_sensor_node_name,
                                         &racobit_radar);
  AdapterManager::PublishRacobitRadar(racobit_radar);
}

// Send the error to monitor and return it
Status RacobitRadarCanbus::OnError(const std::string &error_msg) {
  apollo::common::monitor::MonitorLogBuffer buffer(&monitor_logger_);
  buffer.ERROR(error_msg);
  return Status(ErrorCode::CANBUS_ERROR, error_msg);
}

}  // namespace racobit_radar
}  // namespace drivers
}  // namespace apollo
