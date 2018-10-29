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

/*
 * spiral_reference_line_smoother.h
 */

#ifndef MODULES_PLANNING_PLANNER_OPEN_SPACE_WARM_START_PROBLEM_H_
#define MODULES_PLANNING_PLANNER_OPEN_SPACE_WARM_START_PROBLEM_H_

#include <vector>

#include "Eigen/Dense"

#include "modules/planning/planner/open_space/warm_start_ipopt_interface.h"
#include "modules/planning/proto/planning.pb.h"

namespace apollo {
namespace planning {

class WarmStartProblem {
 public:
  explicit WarmStartProblem(std::size_t horizon, float ts, Eigen::MatrixXd x0,
                            Eigen::MatrixXd xF, Eigen::MatrixXd XYbounds);

  virtual ~WarmStartProblem() = default;

  bool Solve(Eigen::MatrixXd* state_result, Eigen::MatrixXd* control_result,
             Eigen::MatrixXd* time_result);

 private:
  // time horizon
  std::size_t horizon_;

  // time interval
  float ts_;

  // start point
  Eigen::MatrixXd x0_;

  // end point
  Eigen::MatrixXd xF_;

  // XY bounds
  Eigen::MatrixXd XYbounds_;
};

}  // namespace planning
}  // namespace apollo

#endif  // MODULES_PLANNING_PLANNER_OPEN_SPACE_WARM_START_PROBLEM_H_
