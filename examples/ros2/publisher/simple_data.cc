#include <cactus_rt/config.h>
#include <cactus_rt/cyclic_thread.h>
#include <cactus_rt/ros2/app.h>
#include <quill/detail/LogMacros.h>

#include <chrono>
#include <memory>
#include <rclcpp/qos.hpp>
#include <std_msgs/msg/int64.hpp>

#include "cactus_rt/ros2/ros2_adapter.h"

using RealtimeType = std_msgs::msg::Int64;
using RosType = std_msgs::msg::Int64;

class RTROS2PublisherThread : public cactus_rt::CyclicThread, public cactus_rt::ros2::Ros2ThreadMixin {
  int64_t last_published_at_ = 0;
  int64_t run_duration_;

  // We force turn off the trivial data check, because the ROS message data type
  // has a defined constructor with code in it. That said, the code really is
  // pretty trivial so it is safe to use in real-time. We thus disable the trivial
  // type check manually.
  std::shared_ptr<cactus_rt::ros2::Publisher<RealtimeType, RosType, false>> publisher_;

  static cactus_rt::CyclicThreadConfig CreateThreadConfig() {
    cactus_rt::CyclicThreadConfig thread_config;
    thread_config.period_ns = 1'000'000;
    thread_config.cpu_affinity = std::vector<size_t>{2};
    thread_config.SetFifoScheduler(80);

    return thread_config;
  }

 public:
  explicit RTROS2PublisherThread(std::chrono::nanoseconds run_duration = std::chrono::seconds(30))
      : cactus_rt::CyclicThread("RTROS2Publisher", CreateThreadConfig()),
        run_duration_(run_duration.count()) {}

  void InitializeForRos2() override {
    publisher_ = ros2_adapter_->CreatePublisher<RealtimeType, RosType, false>("/cactus_rt/simple", rclcpp::QoS(100));
  }

 protected:
  bool Loop(int64_t elapsed_ns) noexcept override {
    if (elapsed_ns - last_published_at_ > 10'000'000) {
      last_published_at_ = elapsed_ns;

      const auto span = Tracer().WithSpan("Publish");

      std_msgs::msg::Int64 msg;
      msg.data = elapsed_ns;
      const auto success = publisher_->Publish(msg);
      LOG_INFO(Logger(), "{} integer {}", success ? "Published" : "Did not publish", msg.data);
    }

    return elapsed_ns > run_duration_;
  }
};

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);

  cactus_rt::AppConfig app_config;

  cactus_rt::ros2::Ros2Adapter::Config ros2_adapter_config;
  ros2_adapter_config.timer_interval = std::chrono::milliseconds(50);

  cactus_rt::ros2::App app("SimpleDataROS2Publisher", app_config, ros2_adapter_config);
  app.StartTraceSession("build/publisher.perfetto");

  constexpr std::chrono::seconds time(30);
  std::cout << "Testing RT loop for " << time.count() << " seconds.\n";

  auto thread = app.CreateROS2EnabledThread<RTROS2PublisherThread>(time);
  app.RegisterThread(thread);

  app.Start();

  std::this_thread::sleep_for(time);

  app.RequestStop();
  app.Join();

  std::cout << "Done\n";
  return 0;
}
