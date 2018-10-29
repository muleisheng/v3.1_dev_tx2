/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
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

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "boost/format.hpp"
#include "gtest/gtest.h"

#include "modules/common/log.h"
#include "modules/perception/obstacle/fusion/probabilistic_fusion/pbf_sensor_manager.h"
#include "modules/perception/obstacle/fusion/probabilistic_fusion/pbf_sensor_object.h"

#define protected public
#include "modules/perception/obstacle/fusion/probabilistic_fusion/pbf_track.h"

namespace apollo {
namespace perception {

TEST(PbfTrackTest, test_pbf_track_constructor) {
  DSTInitiator::instance().initialize_bba_manager();
  std::shared_ptr<PbfSensorObject> object1(new PbfSensorObject());
  object1->sensor_type = SensorType::VELODYNE_64;
  object1->sensor_id = "velodyne_64";
  object1->timestamp = 0.1;
  object1->object->track_id = 1;
  std::shared_ptr<PbfSensorObject> object2(new PbfSensorObject());
  object2->sensor_type = SensorType::RADAR;
  object2->sensor_id = "radar";
  object2->timestamp = 0.2;
  object2->object->track_id = 2;
  PbfTrack::s_track_idx_ = 0;
  PbfTrack track1(object1);
  PbfTrack track2(object2);
  CHECK_EQ(track1.GetTrackId(), 0);
  CHECK_EQ(track1.IsDead(), false);
  CHECK_EQ(track1.AbleToPublish(), true);
  CHECK_DOUBLE_EQ(track1.GetFusedTimestamp(), 0.1);
  CHECK_EQ(track2.GetTrackId(), 1);
  CHECK_EQ(track2.IsDead(), false);
}

TEST(PbfTrackTest, test_pbf_get_object) {
  DSTInitiator::instance().initialize_bba_manager();
  std::shared_ptr<PbfSensorObject> object1(new PbfSensorObject());
  object1->sensor_type = SensorType::VELODYNE_64;
  object1->sensor_id = "velodyne_64";
  object1->timestamp = 0.1;
  object1->object->track_id = 1;
  PbfTrack track(object1);
  std::shared_ptr<PbfSensorObject> object2(new PbfSensorObject());
  object2->sensor_type = SensorType::RADAR;
  object2->sensor_id = "radar";
  object2->timestamp = 0.09;
  object2->object->track_id = 1;
  track.radar_objects_[object2->sensor_id] = object2;
  CHECK_EQ(nullptr != track.GetLidarObject("velodyne_64"), true);
  CHECK_EQ(nullptr != track.GetRadarObject("radar"), true);

  std::shared_ptr<PbfSensorObject> object4(new PbfSensorObject());
  object4->sensor_type = SensorType::VELODYNE_64;
  object4->sensor_id = "velodyne_64_1";
  object4->timestamp = 0.2;
  object4->object->track_id = 1;
  track.lidar_objects_[object4->sensor_id] = object4;
  std::shared_ptr<PbfSensorObject> object5(new PbfSensorObject());
  object5->sensor_type = SensorType::RADAR;
  object5->sensor_id = "radar_1";
  object5->timestamp = 0.095;
  object5->object->track_id = 1;
  track.radar_objects_[object5->sensor_id] = object5;
  std::shared_ptr<PbfSensorObject> obj = track.GetLatestLidarObject();
  CHECK_EQ(obj->timestamp - 0.2 < 0.001, true);
  obj = track.GetLatestRadarObject();
  CHECK_EQ(obj->timestamp - 0.095 < 0.0001, true);
}

TEST(PbfTrackTest, test_pbf_update_measurements_life) {
  DSTInitiator::instance().initialize_bba_manager();
  std::shared_ptr<PbfSensorObject> object1(new PbfSensorObject());
  object1->sensor_type = SensorType::VELODYNE_64;
  object1->sensor_id = "velodyne_64";
  object1->timestamp = 0.1;
  object1->object->track_id = 1;
  object1->invisible_period = 0.0;
  PbfTrack track(object1);
  std::shared_ptr<PbfSensorObject> object2(new PbfSensorObject());
  object2->sensor_type = SensorType::VELODYNE_64;
  object2->sensor_id = "velodyne_64_1";
  object2->timestamp = 0.1;
  object2->object->track_id = 1;
  track.lidar_objects_[object2->sensor_id] = object2;
  track.UpdateMeasurementsLifeWithMeasurement(&(track.lidar_objects_),
                                              "velodyne_64", 0.45, 0.2);
  CHECK_EQ(track.lidar_objects_.size(), 1);
  track.UpdateMeasurementsLifeWithMeasurement(&(track.lidar_objects_),
                                              "velodyne_64_1", 0.25, 0.2);
  CHECK_EQ(track.lidar_objects_.size(), 1);
  std::shared_ptr<PbfSensorObject> object3(new PbfSensorObject());
  object3->sensor_type = SensorType::VELODYNE_64;
  object3->sensor_id = "velodyne_64_2";
  object3->timestamp = 0.2;
  object3->object->track_id = 1;
  track.lidar_objects_[object3->sensor_id] = object3;
  bool invisible_state = true;
  track.UpdateMeasurementsLifeWithoutMeasurement(
      (&track.lidar_objects_), "velodyne_64", 0.35, 0.2, &invisible_state);
  CHECK_EQ(track.lidar_objects_.size(), 1);
  CHECK_EQ(invisible_state, false);
}

std::shared_ptr<PbfSensorObject> GenerateVelObject(double timestamp) {
  std::shared_ptr<PbfSensorObject> object1(new PbfSensorObject());
  object1->sensor_type = SensorType::VELODYNE_64;
  object1->sensor_id = "velodyne_64";
  object1->timestamp = timestamp;
  object1->object->track_id = 1;
  object1->object->type_probs[static_cast<int>(ObjectType::PEDESTRIAN)] = 1.0f;
  return object1;
}

TEST(PbfTrackClassFusionTest, test_pbf_type_fusion) {
  DSTInitiator::instance().initialize_bba_manager();
  std::shared_ptr<PbfSensorObject> object1 = GenerateVelObject(0.1);
  PbfTrack track(object1);
  BBA* bba = track.GetFusedBBA();
  ADEBUG << "bba print data" << bba->print_bba();
  // update track with camera unknown object
  track.UpdateWithoutSensorObject(SensorType::CAMERA, "1", 0, 0.2);
  bba = track.GetFusedBBA();
  double ts = 0.3;
  for (int i = 0; i < 10; ++i) {
    std::shared_ptr<PbfSensorObject> obj = GenerateVelObject(ts);
    track.UpdateWithSensorObject(obj, 0);
    bba = track.GetFusedBBA();
    ADEBUG << "bba print data" << bba->print_bba();
    ts += 0.1;
    track.UpdateWithoutSensorObject(SensorType::CAMERA, "1", 0, ts);
    bba = track.GetFusedBBA();
    ADEBUG << "bba print bba " << bba->print_bba();
    ts += 0.1;
  }
}

}  // namespace perception
}  // namespace apollo
