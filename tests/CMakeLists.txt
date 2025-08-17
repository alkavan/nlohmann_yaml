cmake_minimum_required(VERSION 3.31)

add_executable(nlohmann_yaml_test nlohmann_yaml_test.cpp)

target_link_libraries(nlohmann_yaml_test PRIVATE nlohmann_yaml::nlohmann_yaml)

# Copy test.yaml to bin directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.yaml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
