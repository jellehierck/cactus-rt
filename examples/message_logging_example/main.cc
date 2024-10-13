#include <cactus_rt/rt.h>

#include <chrono>
#include <iostream>

#include "MultipleLoggersThread.h"
#include "quill/sinks/JsonFileSink.h"

using cactus_rt::App;

int main() {

  // Create a cactus_rt app configuration
  cactus_rt::AppConfig app_config;

  // Create a Quill backend logging config to configure the Quill backend thread
  quill::BackendOptions logger_backend_options = cactus_rt::logging::DefaultBackendOptions();

  // Make sure that the timestamps of each log are in order - although this introduces a small delay on each log
  logger_backend_options.log_timestamp_ordering_grace_period = std::chrono::microseconds(1);

  // Set the background logging thread CPU affinity
  logger_backend_options.cpu_affinity = 1;  // Different CPU than the CyclicThread CPU!

  app_config.logger_backend_options = logger_backend_options;
  App app("LoggingExampleApp", app_config);

  // Define output file names
  std::string csv_output_filename = "data.csv";
  std::string extra_log_output_filename = "extra.json";

  // Define extra logger file sink. For this example, we use a JSON logger.
  quill::JsonFileSinkConfig json_file_sink_config;
  json_file_sink_config.set_open_mode('w');                                                      // Replace contents of current file
  json_file_sink_config.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);  // Append date and time to log filename
  auto json_file_sink = cactus_rt::logging::Frontend::create_or_get_sink<quill::JsonFileSink>(extra_log_output_filename, json_file_sink_config);

  // Define empty logging format, since we will write the data to JSON the format is not used anyways.
  auto empty_pattern = quill::PatternFormatterOptions("");

  // Create extra logger
  cactus_rt::logging::Logger* extra_logger = cactus_rt::logging::Frontend::create_or_get_logger("ExtraLogger", json_file_sink, empty_pattern);

  // CyclicThread config
  cactus_rt::CyclicThreadConfig thread_config;
  thread_config.period_ns = 1'000'000;
  thread_config.cpu_affinity = std::vector<size_t>{2};
  thread_config.SetFifoScheduler(80);

  // Create the thread
  auto thread = app.CreateThread<MultipleLoggersThread>("MultipleLoggersThread", thread_config, extra_logger->get_logger_name(), csv_output_filename, 3'000);

  // Start the app
  constexpr unsigned int time = 5;

  std::cout << "Testing RT loop for " << time << " seconds.\n";

  app.Start();
  std::this_thread::sleep_for(std::chrono::seconds(time));
  app.RequestStop();
  app.Join();

  std::cout << "Number of loops executed: " << thread->GetLoopCounter() << "\n";
  return 0;
}
