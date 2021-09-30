// Copyright (C) 2021 The Android Open Source Project
// Copyright (C) 2021 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <inttypes.h>
#include <memory>
#include <variant>

// Library to log metrics.
namespace android {
namespace base {

// Events that can be logged.
struct MetricEventFreeze {};
struct MetricEventUnFreeze { int64_t frozen_ms; };

using MetricEventType = std::variant<std::monostate, MetricEventFreeze, MetricEventUnFreeze>;

class MetricsLogger {
public:
    // Log a MetricEventType.
    virtual void logMetricEvent(MetricEventType eventType) = 0;
    // Virtual destructor.
    virtual ~MetricsLogger() = default;
};

std::unique_ptr<MetricsLogger> CreateMetricsLogger();

}  // namespace base
}  // namespace android