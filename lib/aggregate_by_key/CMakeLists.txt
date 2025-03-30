add_library(
  aggregate_by_key_lib
  INTERFACE
)

target_include_directories(
  aggregate_by_key_lib
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  aggregate_by_key
  INTERFACE
  as_data_flow_lib
)