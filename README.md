# nlohmann_yaml

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Header Only](https://img.shields.io/badge/Header--Only-✓-brightgreen.svg)](https://github.com/yourusername/nlohmann_yaml)
[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A header-only YAML parser that integrates [Nlohmann's JSON](https://github.com/nlohmann/json/) library,
providing a simple way to parse YAML files into familiar JSON objects.

## Features

- **Header-Only**: No compilation required, simply `#include <nlohmann/yaml.hpp>` 
- **Efficient Parsing**: Optimized for performance with minimal memory overhead
- **Easy Integration**: Direct conversion to `nlohmann::json` objects
- **Type Detection**: Automatic parsing of strings, numbers, booleans, and null values
- **Modern C++**: Designed for C++17 standard and later
- **CMake Support**: Proper CMake package configuration
- **Comment Handling**: Gracefully processes YAML comments
- **Stream Support**: Parse from strings, files, or any `std::istream`

## Getting Started

### Installation

Installation instructions for Linux and Windows are provided below:

#### Linux
```
mkdir build
cd build
cmake ..
cmake --build .
```

#### Windows
```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=c:/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

### CMake Integration

```
target_link_libraries(app PRIVATE nlohmann_yaml::nlohmann_yaml)
```

## Usage

```cpp
#include <nlohmann/yaml.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

nlohmann::json load_yaml(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }

    return nlohmann::parse_yaml(ifs);
}

int main(const int argc, char* argv[]) {
    try {
        nlohmann::json test_json = load_yaml("config.yaml");

        // Print the parsed JSON structure
        std::cout << test_json.dump(2) << std::endl << std::endl;

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
}
```
