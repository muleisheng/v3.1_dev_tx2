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

#ifndef MODULES_PREDICTION_SCENARIO_SCENARIO_MANAGER_H_
#define MODULES_PREDICTION_SCENARIO_SCENARIO_MANAGER_H_

#include "modules/common/macro.h"
#include "modules/common/proto/scenario.pb.h"
#include "modules/prediction/proto/prediction_conf.pb.h"
#include "modules/prediction/scenario/feature_extractor/feature_extractor.h"
#include "modules/prediction/scenario/analyzer/scenario_analyzer.h"

namespace apollo {
namespace prediction {

class ScenarioManager {
 public:
  /**
   * @brief Run scenario analysis
   */
  void Run();

  /**
   * @brief Get scenario analysis result
   */
  const apollo::common::Scenario& scenario() const;

 private:
  FeatureExtractor feature_extractor_;
  ScenarioAnalyzer scenario_analyzer_;

  DECLARE_SINGLETON(ScenarioManager)
};

}  // namespace prediction
}  // namespace apollo

#endif  // MODULES_PREDICTION_SCENARIO_SCENARIO_MANAGER_H_
