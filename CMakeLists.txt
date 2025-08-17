# Copyright (C) 2025 Igal Alkon <igal@alkontek.com> and contributors
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.31)
project(nlohmann_yaml VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Find Nlohmann's JSON library
find_package(nlohmann_json CONFIG REQUIRED)

# Create header-only interface library
add_library(nlohmann_yaml INTERFACE)
add_library(nlohmann_yaml::nlohmann_yaml ALIAS nlohmann_yaml)

# Set linker language for export compatibility
set_target_properties(nlohmann_yaml PROPERTIES LINKER_LANGUAGE CXX)

# Set include directories for the library
target_include_directories(nlohmann_yaml
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(nlohmann_yaml INTERFACE nlohmann_json::nlohmann_json)

# Require C++17 interface
target_compile_features(nlohmann_yaml INTERFACE cxx_std_17)

# Installation
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install the header files
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the library target
install(TARGETS nlohmann_yaml
    EXPORT nlohmann_yamlTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the CMake config files
install(EXPORT nlohmann_yamlTargets
    FILE nlohmann_yamlTargets.cmake
    NAMESPACE nlohmann_yaml::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nlohmann_yaml
)

# Generate and install config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/nlohmann_yamlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nlohmann_yamlConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nlohmann_yaml
)

# Generate and install version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/nlohmann_yamlConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install library assets
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/nlohmann_yamlConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/nlohmann_yamlConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nlohmann_yaml
)

# Tests (only build if this is the main project)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(tests)
endif()
