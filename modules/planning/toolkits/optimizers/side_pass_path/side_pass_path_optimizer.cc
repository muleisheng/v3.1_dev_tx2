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
 **/

#include "modules/planning/toolkits/optimizers/side_pass_path/side_pass_path_optimizer.h"

#include <string>
#include <utility>
#include <vector>

#include "modules/common/util/file.h"
#include "modules/planning/common/planning_gflags.h"
#include "modules/planning/toolkits/optimizers/road_graph/dp_road_graph.h"
#include "modules/planning/toolkits/optimizers/road_graph/side_pass_waypoint_sampler.h"

namespace apollo {
namespace planning {

using apollo::common::ErrorCode;
using apollo::common::Status;

SidePassPathOptimizer::SidePassPathOptimizer()
    : PathOptimizer("SidePassPathOptimizer") {}

bool SidePassPathOptimizer::Init(
    const ScenarioConfig::ScenarioTaskConfig &config) {
  config_ = config.dp_poly_path_config();
  is_init_ = true;
  return true;
}

Status SidePassPathOptimizer::Process(const SpeedData &speed_data,
                                      const ReferenceLine &,
                                      const common::TrajectoryPoint &init_point,
                                      PathData *const path_data) {
  if (!is_init_) {
    AERROR << "Please call Init() before Process().";
    return Status(ErrorCode::PLANNING_ERROR, "Not inited.");
  }
  CHECK_NOTNULL(path_data);
  DpRoadGraph dp_road_graph(config_, *reference_line_info_, speed_data);
  dp_road_graph.SetDebugLogger(reference_line_info_->mutable_debug());
  dp_road_graph.SetWaypointSampler(
      new SidePassWaypointSampler(config_.waypoint_sampler_config()));

  if (!dp_road_graph.FindPathTunnel(
          init_point,
          reference_line_info_->path_decision()->path_obstacles().Items(),
          path_data)) {
    AERROR << "Failed to find tunnel in road graph";
    return Status(ErrorCode::PLANNING_ERROR, "dp_road_graph path generation");
  }

  return Status::OK();
}

}  // namespace planning
}  // namespace apollo
