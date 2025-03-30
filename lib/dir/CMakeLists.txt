add_library(
  dir_lib
  INTERFACE
)

target_include_directories(
  dir_lib
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
