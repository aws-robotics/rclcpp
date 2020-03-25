// Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCLCPP__SUBSCRIBER_TOPIC_STATISTICS_HPP_
#define RCLCPP__SUBSCRIBER_TOPIC_STATISTICS_HPP_

#include <vector>

#include "rcl/time.h"

#include "rclcpp/publisher.hpp"
#include "rclcpp/timer.hpp"

#include "metrics_statistics_msgs/msg/metrics_message.hpp"

#include "topic_statistics_collector/constants.hpp"
#include "topic_statistics_collector/received_message_age.hpp"
#include "topic_statistics_collector/received_message_period.hpp"

#include <iostream>


namespace rclcpp {
namespace topic_statistics {


template<typename CallbackMessageT>
class SubcriberTopicStatistics {

using topic_stats_collector =
  topic_statistics_collector::TopicStatisticsCollector<CallbackMessageT>;
using received_message_age =
  topic_statistics_collector::ReceivedMessageAgeCollector<CallbackMessageT>;
using received_message_period =
  topic_statistics_collector::ReceivedMessagePeriodCollector<CallbackMessageT>;

public:
  //todo doc
  SubcriberTopicStatistics(
    rclcpp::Publisher<metrics_statistics_msgs::msg::MetricsMessage>::SharedPtr & publisher)
    : publisher_(std::move(publisher))
  {

    auto rma = std::make_unique<received_message_age>();
    rma->Start();
    subscriber_statistics_collectors_.emplace_back(std::move(rma));

    auto rmp = std::make_unique<received_message_period>();
    rmp->Start();
    subscriber_statistics_collectors_.emplace_back(std::move(rmp));
  }

  virtual ~SubcriberTopicStatistics() {
    for (auto & collector: subscriber_statistics_collectors_) {
      collector->Stop();
    }
    subscriber_statistics_collectors_.clear();
  }

  // todo doc
  virtual void OnMessageReceived(
    const CallbackMessageT & received_message,
    const rcl_time_point_value_t now_nanoseconds) const {

    (void)received_message;

    for (auto & collector: subscriber_statistics_collectors_) {
      collector->OnMessageReceived(received_message, now_nanoseconds);
      std::cout << "---" << collector->GetStatusString() << "---\n";
    }
  }

  virtual void PublishMessage() {
    std::cout << "===Publishing!===" << "\n\n";
    //todo construct a message
  }

  virtual void TearDown() {
    if (measurement_timer_) {
      measurement_timer_->cancel();
      measurement_timer_.reset();
    }
    if (publisher_) {
      publisher_.reset();
    }
  }

  void SetTimer(const rclcpp::TimerBase::SharedPtr & measurement_timer) {
    measurement_timer_ = measurement_timer;
  }

private:

std::vector<std::unique_ptr<topic_stats_collector>> subscriber_statistics_collectors_{};

rclcpp::Publisher<metrics_statistics_msgs::msg::MetricsMessage>::SharedPtr publisher_{nullptr};
rclcpp::TimerBase::SharedPtr measurement_timer_{nullptr};

};
}  // namespace topic_statistics
}  // namespace rclcpp
#endif //RCLCPP__SUBSCRIBER_TOPIC_STATISTICS_HPP_
