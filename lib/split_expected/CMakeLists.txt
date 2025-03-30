add_library(
  split_expected_lib
  INTERFACE
)

target_include_directories(
  split_expected_lib
  INTERFACE
  ${CMAKE_INCLUDE_CURRENT_DIR}
)