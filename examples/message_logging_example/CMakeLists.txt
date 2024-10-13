add_executable(rt_message_logging_example
  main.cc
  MultipleLoggersThread.cc
)

target_link_libraries(rt_message_logging_example
  PRIVATE
  cactus_rt
)

setup_cactus_rt_target_options(rt_message_logging_example)
