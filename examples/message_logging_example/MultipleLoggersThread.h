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
  // TODO: There seems to be a bug where passing in custom FrontendOptions are not supported (https://github.com/odygrd/quill/issues/609), so temporarily the default are passed instead (but they should not be)
  quill::CsvWriter<MyCsvSchema, quill::FrontendOptions> csv_writer_;
  // quill::CsvWriter<MyCsvSchema, cactus_rt::logging::FrontendOptions> csv_writer_;
};

#endif
