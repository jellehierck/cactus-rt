#include "MultipleLoggersThread.h"

#include <cmath>

#include "quill/Backend.h"
#include "quill/LogMacros.h"

MultipleLoggersThread::MultipleLoggersThread(std::string thread_name, cactus_rt::CyclicThreadConfig thread_config, std::string extra_logger_name, std::string csv_data_filename, size_t max_iterations)
    : CyclicThread(thread_name, thread_config),
      max_iterations_(max_iterations),
      extra_logger_(cactus_rt::logging::Frontend::get_logger(extra_logger_name))
// TODO: The CSV writer has a breaking bug in Quill which will be fixed in 7.4.0 (https://github.com/odygrd/quill/issues/609). It cannot be used with a custom for now, so it is disabled.
// , csv_writer_(csv_data_filename)
{
  if (this->ExtraLogger() == nullptr) {
    LOG_ERROR(this->Logger(), "Could not get logger with name \"{}\"", extra_logger_name);
    // TODO: throw error?
  }
}

MultipleLoggersThread::~MultipleLoggersThread() {
  // Flushing the csv writer only if the background thread is still running,
  // otherwise `flush()` will block indefinitely.
  if (quill::Backend::is_running()) {
    // Blocks until all messages up to the current timestamp are flushed on the
    // csv writer, to ensure every data point is logged.
    // TODO: The CSV writer has a breaking bug in Quill which will be fixed in 7.4.0 (https://github.com/odygrd/quill/issues/609). It cannot be used with a custom for now, so it is disabled.
    // this->csv_writer_.flush();

    // Blocks until all messages up to the current timestamp are flushed on the
    // extra logger, to ensure every message is logged.
    this->ExtraLogger()->flush_log();
  }
}

cactus_rt::CyclicThread::LoopControl MultipleLoggersThread::Loop(int64_t elapsed_ns) noexcept {
  const double elapsed_s = static_cast<double>(elapsed_ns) / 1'000'000'000.0;

  const double period = 1.0;
  const double amplitude = 1.0;
  double       data = amplitude * cos(2 * M_PI / period * elapsed_s);
  std::string  data_status = data > 0.0 ? "positive" : "negative";

  // Log some information to the regular thread logger
  LOG_INFO_LIMIT(std::chrono::milliseconds(500), this->Logger(), "Iterations: {}/{}", iterations_, max_iterations_);

  // Log some information to a JSON logger
  LOGJ_INFO(this->ExtraLogger(), "custom data", iterations_, data_status, data);
  // LOGJ_INFO(this->ExtraLogger(), "", iterations_, data_status, data);

  // Use the data logger to write data to the output file
  // TODO: The CSV writer has a breaking bug in Quill which will be fixed in 7.4.0 (https://github.com/odygrd/quill/issues/609). It cannot be used with a custom for now, so it is disabled.
  // this->csv_writer_.append_row(iterations_, elapsed_s, data);

  ++iterations_;
  return iterations_ >= max_iterations_ ? LoopControl::Stop : LoopControl::Continue;
}
