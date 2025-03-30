add_library(
  out_lib
  INTERFACE
)

target_include_directories(
  out_lib
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)