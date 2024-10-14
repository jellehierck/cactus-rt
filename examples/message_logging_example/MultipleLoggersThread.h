#ifndef MESSAGE_LOGGING_EXAMPLE_MULTIPLELOGGERTHREAD_H_
#define MESSAGE_LOGGING_EXAMPLE_MULTIPLELOGGERTHREAD_H_

#include <cactus_rt/rt.h>

#include <string>

#include "quill/CsvWriter.h"

/**
 * Define a CSV schema at compile time.
 * https://quillcpp.readthedocs.io/en/latest/csv_writing.html
 */
struct MyCsvSchema {
  static constexpr char const* header = "iteration,time,data";
  static constexpr char const* format = "{},{:6f},{:6f}";
};

class MultipleLoggersThread : public cactus_rt::CyclicThread {
 public:
  MultipleLoggersThread(std::string thread_name, cactus_rt::CyclicThreadConfig thread_config, std::string extra_logger_name, std::string csv_data_filename, size_t max_iterations = 30000);

  ~MultipleLoggersThread() override;

  size_t GetLoopCounter() const { return iterations_; }

 protected:
  LoopControl Loop(int64_t elapsed_ns) noexcept final;

  cactus_rt::logging::Logger* ExtraLogger() const { return extra_logger_; };

 private:
  size_t max_iterations_;
  size_t iterations_ = 0;

  cactus_rt::logging::Logger* extra_logger_;

  // !! Make sure to use cactus_rt::logging::FrontendOptions and not the default quill::FrontendOptions !!
  // TODO: The CSV writer has a breaking bug in Quill which will be fixed in 7.4.0 (https://github.com/odygrd/quill/issues/609). It cannot be used with a custom for now, so it is disabled.
  // quill::CsvWriter<MyCsvSchema, cactus_rt::logging::FrontendOptions> csv_writer_;
};

#endif
