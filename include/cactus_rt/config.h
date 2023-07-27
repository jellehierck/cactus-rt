#ifndef CACTUS_RT_CONFIG_H_
#define CACTUS_RT_CONFIG_H_

#include <variant>

#include "quill/Quill.h"

namespace cactus_rt {

/**
 * @brief The configuration required for an App
 */
struct AppConfig {
  // The name of the app
  char* name;

  // Configuration for quill logging
  quill::Config logger_config;

  // Size of heap to reserve in bytes at program startup.
  size_t heap_size = 0;
};

/**
 * @brief The configuration required for an SCHED_OTHER thread
 */
struct OtherThreadConfig {
  int32_t nice = 0;
};

/**
 * @brief The configuration required for a SCHED_FIFO thread
 */
struct FifoThreadConfig {
  uint32_t priority = 0;
};

/**
 * @brief The configuration required for a SCHED_DEADLINE thread
 */
struct DeadlineThreadConfig {
  uint64_t sched_runtime_ns;
  uint64_t sched_deadline_ns;
  uint64_t sched_period_ns;
};

/**
 * @brief The configuration required for a thread
 */
struct ThreadConfig {
  // The name of the thread
  std::string name;

  // A vector of CPUs this thread should run on. If empty, no CPU restrictions are set.
  std::vector<size_t> cpu_affinity = {};

  // The size of the stack for this thread. Defaults to 8MB.
  size_t stack_size = 8 * 1024 * 1024;

  // The configuration for the scheduler (SCHED_OTHER, SCHED_FIFO, or SCHED_DEADLINE)
  std::variant<OtherThreadConfig, FifoThreadConfig, DeadlineThreadConfig> scheduler_config;
};

struct CyclicThreadConfig : ThreadConfig {
  // The period of the cyclic thread in ns
  uint64_t period_ns = 1'000'000;
};

}  // namespace cactus_rt

#endif
