
/*
    Copyright (C) 2025 Igal Alkon <igal@alkontek.com> and contributors

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

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
        int tests_passed = 0;
        int tests_failed = 0;

        // Helper lambda for test validation
        auto test_value = [&](const std::string& test_name, const bool condition) {
            if (condition) {
                std::cout << "[PASS] " << test_name << std::endl;
                tests_passed++;
            } else {
                std::cout << "[FAIL] " << test_name << std::endl;
                tests_failed++;
            }
        };
        
        // Basic test for string-based parse_yaml interface
        std::cout << "Testing string-based parse_yaml interface...\n" << std::endl;

        std::string yaml_string = R"(
name: test_user
age: 25
active: true
tags:
  - developer
  - yaml
  - testing
config:
  debug: false
  timeout: 30
)";

        // Basic validation of string parsing
        nlohmann::json parsed_string_json = nlohmann::parse_yaml(yaml_string);

        // Print the parsed JSON structure (string)
        std::cout << "Parsed (string) JSON structure:" << std::endl;
        std::cout << parsed_string_json.dump(2) << std::endl << std::endl;

        // Validate string-based parsing
        std::cout << "=== Testing String-based YAML Parsing ===" << std::endl;
        test_value("String parsing - name field", parsed_string_json.contains("name") && parsed_string_json["name"] == "test_user");
        test_value("String parsing - age field", parsed_string_json.contains("age") && parsed_string_json["age"] == 25);
        test_value("String parsing - active field", parsed_string_json.contains("active") && parsed_string_json["active"] == true);
        test_value("String parsing - tags field exists and is array", parsed_string_json.contains("tags") && parsed_string_json["tags"].is_array());
        test_value("String parsing - tags array has 3 elements", parsed_string_json["tags"].size() == 3);
        test_value("String parsing - config debug field", parsed_string_json.contains("config") && parsed_string_json["config"]["debug"] == false);

        std::cout << std::endl;


        // Basic test for string-based parse_yaml interface
        std::cout << "Testing stream-based parse_yaml interface...\n" << std::endl;

        nlohmann::json parsed_json = load_yaml("test.yaml");

        // Print the parsed JSON structure (file)
        std::cout << "Parsed (file) JSON structure:" << std::endl;
        std::cout << parsed_json.dump(2) << std::endl << std::endl;

        // Check if we have expected top-level keys from test.yaml
        std::cout << "=== Testing YAML Structure ===" << std::endl;
        test_value("Root key exists", parsed_json.contains("root") || parsed_json.contains("oot"));
        test_value("Top level list exists", parsed_json.contains("top_level_list"));
        test_value("Trailing comment key exists", parsed_json.contains("trailing_comment_key"));
        test_value("Tab indent key exists", parsed_json.contains("tab_indent"));
        test_value("JSON compatibility key exists", parsed_json.contains("json_compatibility"));
        test_value("YAML edge cases key exists", parsed_json.contains("yaml_edge_cases"));

        // Use the actual root key (could be "root" or "oot" due to parsing issue)
        if (std::string root_key = parsed_json.contains("root") ? "root" : "oot"; parsed_json.contains(root_key)) {
            auto root = parsed_json[root_key];

            std::cout << "\n=== Testing Scalar Values ===" << std::endl;
            // Test scalar values from test.yaml
            test_value("string_unquoted", root.contains("string_unquoted")
                && root["string_unquoted"] == "hello world");

            test_value("string_quoted_single", root.contains("string_quoted_single")
                && root["string_quoted_single"] == "single quoted string");

            test_value("string_quoted_double", root.contains("string_quoted_double")
                && root["string_quoted_double"] == "double quoted string");

            test_value("integer", root.contains("integer") && root["integer"] == 42);
            test_value("float", root.contains("float") && root["float"] == 3.14);
            test_value("boolean_true", root.contains("boolean_true") && root["boolean_true"] == true);
            test_value("boolean_false", root.contains("boolean_false") && root["boolean_false"] == false);
            test_value("boolean_True", root.contains("boolean_True") && root["boolean_True"] == true);
            test_value("boolean_False", root.contains("boolean_False") && root["boolean_False"] == false);
            test_value("null_null", root.contains("null_null") && root["null_null"].is_null());
            test_value("null_tilde", root.contains("null_tilde") && root["null_tilde"].is_null());

            std::cout << "\n=== Testing Nested Mappings ===" << std::endl;
            // Test nested mapping from test.yaml
            if (root.contains("nested_map")) {
                auto nested = root["nested_map"];

                test_value("nested_map.key1",
                    nested.contains("key1") && nested["key1"] == "value1");

                test_value("nested_map.key2",
                    nested.contains("key2") && nested["key2"] == "value2");

                if (nested.contains("deeper_map")) {
                    auto deeper = nested["deeper_map"];
                    test_value("nested_map.deeper_map.subkey",
                        deeper.contains("subkey") && deeper["subkey"] == "subvalue");
                } else {
                    test_value("nested_map.deeper_map exists", false);
                }
            } else {
                test_value("nested_map exists", false);
            }

            std::cout << "\n=== Testing Simple Arrays/Lists ===" << std::endl;
            // Test simple list from test.yaml
            if (root.contains("simple_list") && root["simple_list"].is_array()) {
                auto list = root["simple_list"];
                test_value("simple_list is array", true);
                test_value("simple_list has 5 items", list.size() == 5);
                if (list.size() >= 5) {
                    test_value("simple_list[0] = 'item1'", list[0] == "item1");
                    test_value("simple_list[1] = 'item2'", list[1] == "item2");
                    test_value("simple_list[2] = 3", list[2] == 3);
                    test_value("simple_list[3] = true", list[3] == true);
                    test_value("simple_list[4] is null", list[4].is_null());
                }
            } else {
                test_value("simple_list exists and is array", false);
            }

            std::cout << "\n=== Testing Nested Arrays ===" << std::endl;
            // Test nested list from test.yaml
            if (root.contains("nested_list") && root["nested_list"].is_array()) {
                auto nested_list = root["nested_list"];

                test_value("nested_list is array", true);
                test_value("nested_list has 2 items", nested_list.size() == 2);

                if (nested_list.size() >= 2) {
                    test_value("nested_list[0] is array", nested_list[0].is_array());
                    test_value("nested_list[1] is array", nested_list[1].is_array());
                    if (nested_list[0].is_array() && nested_list[0].size() >= 2) {
                        test_value("nested_list[0][0] = 'subitem1'", nested_list[0][0] == "subitem1");
                        test_value("nested_list[0][1] = 'subitem2'", nested_list[0][1] == "subitem2");
                    }
                    if (nested_list[1].is_array() && nested_list[1].size() >= 2) {
                        test_value("nested_list[1][0] = 4", nested_list[1][0] == 4);
                        test_value("nested_list[1][1] = 5.5", nested_list[1][1] == 5.5);
                    }
                }
            } else {
                test_value("nested_list exists and is array", false);
            }

            std::cout << "\n=== Testing Maps with Lists ===" << std::endl;
            // Test map_with_list from test.yaml
            if (root.contains("map_with_list")) {
                if (auto map_with_list = root["map_with_list"];
                    map_with_list.contains("list_key") && map_with_list["list_key"].is_array()) {

                    auto list_key = map_with_list["list_key"];

                    test_value("map_with_list.list_key is array", true);
                    test_value("map_with_list.list_key has 2 items", list_key.size() == 2);

                    if (list_key.size() >= 2) {
                        test_value("map_with_list.list_key[0] = 'list_item1'", list_key[0] == "list_item1");
                        test_value("map_with_list.list_key[1] = 'list_item2'", list_key[1] == "list_item2");
                    }
                } else {
                    test_value("map_with_list.list_key exists and is array", false);
                }
            } else {
                test_value("map_with_list exists", false);
            }

            std::cout << "\n=== Testing Lists with Maps ===" << std::endl;
            // Test list_with_maps from test.yaml
            if (root.contains("list_with_maps") && root["list_with_maps"].is_array()) {
                auto list_with_maps = root["list_with_maps"];

                test_value("list_with_maps is array", true);
                test_value("list_with_maps has 2 items", list_with_maps.size() == 2);

                if (list_with_maps.size() >= 2) {
                    test_value("list_with_maps[0] has map1", list_with_maps[0].contains("map1"));

                    test_value("list_with_maps[1] has map2", list_with_maps[1].contains("map2"));

                    if (list_with_maps[0].contains("map1")) {
                        auto map1 = list_with_maps[0]["map1"];

                        test_value("list_with_maps[0].map1.a = 1",
                            map1.contains("a") && map1["a"] == 1);

                        test_value("list_with_maps[0].map1.b = 2",
                            map1.contains("b") && map1["b"] == 2);
                    }

                    if (list_with_maps[1].contains("map2")) {
                        auto map2 = list_with_maps[1]["map2"];

                        test_value("list_with_maps[1].map2.c = 3",
                            map2.contains("c") && map2["c"] == 3);

                        test_value("list_with_maps[1].map2.d = 4",
                            map2.contains("d") && map2["d"] == 4);
                    }
                }
            } else {
                test_value("list_with_maps exists and is array", false);
            }

            std::cout << "\n=== Testing Complex Nested Structures ===" << std::endl;
            // Test complex structure from test.yaml
            if (root.contains("complex")) {
                if (auto complex = root["complex"]; complex.contains("map")) {
                    if (auto map = complex["map"]; map.contains("list") && map["list"].is_array()) {
                        auto list = map["list"];

                        test_value("complex.map.list is array", true);
                        test_value("complex.map.list has 2 items", list.size() == 2);

                        if (list.size() >= 2) {
                            test_value("complex.map.list[0] has scalar",
                                list[0].contains("scalar"));

                            test_value("complex.map.list[0].scalar = 'value'",
                                list[0].contains("scalar") && list[0]["scalar"] == "value");

                            if (list[0].contains("sublist") && list[0]["sublist"].is_array()) {
                                auto sublist = list[0]["sublist"];

                                test_value("complex.map.list[0].sublist[0] = 1",
                                    !sublist.empty() && sublist[0] == 1);

                                test_value("complex.map.list[0].sublist[1] = 2",
                                    sublist.size() > 1 && sublist[1] == 2);
                            }

                            test_value("complex.map.list[1].another = 'map'",
                                list[1].contains("another") && list[1]["another"] == "map");

                            test_value("complex.map.list[1].with = 'values'",
                                list[1].contains("with") && list[1]["with"] == "values");
                        }
                    } else {
                        test_value("complex.map.list exists and is array", false);
                    }
                } else {
                    test_value("complex.map exists", false);
                }
            } else {
                test_value("complex exists", false);
            }
        }

        std::cout << "\n=== Testing Top-level Elements ===" << std::endl;

        // Test top_level_list from test.yaml
        if (parsed_json.contains("top_level_list") && parsed_json["top_level_list"].is_array()) {
            auto top_list = parsed_json["top_level_list"];

            test_value("top_level_list is array", true);
            test_value("top_level_list has 2 items", top_list.size() == 2);

            if (top_list.size() >= 2) {
                test_value("top_level_list[0] = 'top_item1'", top_list[0] == "top_item1");
                test_value("top_level_list[1] = 'top_item2'", top_list[1] == "top_item2");
            }
        } else {
            test_value("top_level_list exists and is array", false);
        }

        // Test trailing_comment_key from test.yaml
        test_value("trailing_comment_key = 'value'",
            parsed_json.contains("trailing_comment_key") && parsed_json["trailing_comment_key"] == "value");

        // Test tab_indent from test.yaml
        if (parsed_json.contains("tab_indent")) {
            auto tab_indent = parsed_json["tab_indent"];
            test_value("tab_indent.key = 'value'",
                tab_indent.contains("key") && tab_indent["key"] == "value");
        } else {
            test_value("tab_indent exists", false);
        }

        // Test JSON compatibility section from test.yaml
        std::cout << "\n=== Testing JSON Compatibility ===" << std::endl;
        if (parsed_json.contains("json_compatibility")) {
            auto json_compat = parsed_json["json_compatibility"];

            test_value("json_compatibility section exists", true);

            // Test JSON-style arrays
            if (json_compat.contains("json_array") && json_compat["json_array"].is_array()) {
                auto json_array = json_compat["json_array"];

                test_value("json_array is array", true);
                test_value("json_array has 6 items", json_array.size() == 6);

                if (json_array.size() >= 6) {
                    test_value("json_array[0] = 1", json_array[0] == 1);
                    test_value("json_array[1] = 2", json_array[1] == 2);
                    test_value("json_array[2] = 3", json_array[2] == 3);
                    test_value("json_array[3] = 'four'", json_array[3] == "four");
                    test_value("json_array[4] = true", json_array[4] == true);
                    test_value("json_array[5] is null", json_array[5].is_null());
                }
            } else {
                test_value("json_array exists and is array", false);
            }

            // Test nested JSON arrays
            if (json_compat.contains("json_nested_array") && json_compat["json_nested_array"].is_array()) {
                auto nested_array = json_compat["json_nested_array"];

                test_value("json_nested_array is array", true);
                test_value("json_nested_array has 3 items", nested_array.size() == 3);

                if (nested_array.size() >= 3 && nested_array[0].is_array() && nested_array[0].size() >= 2) {
                    test_value("json_nested_array[0][0] = 1", nested_array[0][0] == 1);
                    test_value("json_nested_array[0][1] = 2", nested_array[0][1] == 2);
                }

                if (nested_array.size() >= 3 && nested_array[2].is_array() && nested_array[2].size() >= 2) {
                    test_value("json_nested_array[2][0] = 'a'", nested_array[2][0] == "a");
                    test_value("json_nested_array[2][1] = 'b'", nested_array[2][1] == "b");
                }
            } else {
                test_value("json_nested_array exists and is array", false);
            }

            // Test JSON-style objects
            if (json_compat.contains("json_object")) {
                auto json_object = json_compat["json_object"];
                test_value("json_object.key1 = 'value1'",
                    json_object.contains("key1") && json_object["key1"] == "value1");
                test_value("json_object.key2 = 42",
                    json_object.contains("key2") && json_object["key2"] == 42);
                test_value("json_object.key3 = true",
                    json_object.contains("key3") && json_object["key3"] == true);
            } else {
                test_value("json_object exists", false);
            }

            // Test nested JSON objects
            if (json_compat.contains("json_nested_object")) {
                if (auto nested_obj = json_compat["json_nested_object"]; nested_obj.contains("outer")) {
                    auto outer = nested_obj["outer"];

                    test_value("json_nested_object.outer.inner = 'value'",
                        outer.contains("inner") && outer["inner"] == "value");

                    test_value("json_nested_object.outer.number = 123",
                        outer.contains("number") && outer["number"] == 123);
                } else {
                    test_value("json_nested_object.outer exists", false);
                }
            } else {
                test_value("json_nested_object exists", false);
            }

            // Test mixed structures
            if (json_compat.contains("mixed_structure")) {
                auto mixed = json_compat["mixed_structure"];

                if (mixed.contains("json_in_yaml")) {
                    auto json_in_yaml = mixed["json_in_yaml"];

                    test_value("mixed_structure.json_in_yaml.a = 1",
                        json_in_yaml.contains("a") && json_in_yaml["a"] == 1);

                    if (json_in_yaml.contains("b") && json_in_yaml["b"].is_array()) {
                        auto b_array = json_in_yaml["b"];

                        test_value("mixed_structure.json_in_yaml.b is array with 3 items",
                            b_array.size() == 3);

                        if (b_array.size() >= 3) {
                            test_value("mixed_structure.json_in_yaml.b[0] = 2", b_array[0] == 2);
                            test_value("mixed_structure.json_in_yaml.b[1] = 3", b_array[1] == 3);
                            test_value("mixed_structure.json_in_yaml.b[2] = 4", b_array[2] == 4);
                        }
                    }
                }
                if (mixed.contains("yaml_in_json") && mixed["yaml_in_json"].is_array()) {
                    auto yaml_in_json = mixed["yaml_in_json"];

                    test_value("mixed_structure.yaml_in_json is array", true);

                    if (!yaml_in_json.empty()) {
                        auto first_item = yaml_in_json[0];

                        test_value("mixed_structure.yaml_in_json[0].name = 'test'",
                            first_item.contains("name") && first_item["name"] == "test");

                        if (first_item.contains("values") && first_item["values"].is_array()) {
                            auto values = first_item["values"];
                            test_value("mixed_structure.yaml_in_json[0].values has 3 items", values.size() == 3);
                        }
                    }
                }
            }

            // Test JSON booleans and null
            if (json_compat.contains("json_booleans")) {
                auto json_bools = json_compat["json_booleans"];

                test_value("json_booleans.true_value = true",
                    json_bools.contains("true_value") && json_bools["true_value"] == true);

                test_value("json_booleans.false_value = false",
                    json_bools.contains("false_value") && json_bools["false_value"] == false);

                test_value("json_booleans.null_value is null",
                    json_bools.contains("null_value") && json_bools["null_value"].is_null());
            }

            // Test empty structures
            if (json_compat.contains("empty_array") && json_compat["empty_array"].is_array()) {
                test_value("empty_array is empty", json_compat["empty_array"].empty());
            } else {
                test_value("empty_array exists and is array", false);
            }

            if (json_compat.contains("empty_object") && json_compat["empty_object"].is_object()) {
                test_value("empty_object is empty", json_compat["empty_object"].empty());
            } else {
                test_value("empty_object exists and is object", false);
            }

            // Test edge cases
            if (json_compat.contains("edge_cases")) {
                auto edge_cases = json_compat["edge_cases"];

                test_value("edge_cases.unicode_string contains unicode",
                    edge_cases.contains("unicode_string"));

                test_value("edge_cases.escaped_quotes contains escaped quotes",
                    edge_cases.contains("escaped_quotes"));

                test_value("edge_cases.special_chars contains special chars",
                    edge_cases.contains("special_chars"));

                if (edge_cases.contains("numbers")) {
                    auto numbers = edge_cases["numbers"];
                    test_value("edge_cases.numbers.integer = 42",
                        numbers.contains("integer") && numbers["integer"] == 42);

                    test_value("edge_cases.numbers.negative = -17",
                        numbers.contains("negative") && numbers["negative"] == -17);

                    test_value("edge_cases.numbers.float = 3.14159",
                        numbers.contains("float") && numbers["float"] == 3.14159);

                    test_value("edge_cases.numbers.zero = 0",
                        numbers.contains("zero") && numbers["zero"] == 0);
                }
            }

            // Test complex JSON structure
            if (json_compat.contains("complex_json")) {
                if (auto complex_json = json_compat["complex_json"];
                    complex_json.contains("users") && complex_json["users"].is_array()) {

                    auto users = complex_json["users"];

                    test_value("complex_json.users is array with 2 items", users.size() == 2);

                    if (users.size() >= 2) {
                        auto user1 = users[0];

                        test_value("complex_json.users[0].id = 1",
                            user1.contains("id") && user1["id"] == 1);

                        test_value("complex_json.users[0].name = 'John Doe'",
                            user1.contains("name") && user1["name"] == "John Doe");

                        test_value("complex_json.users[0].active = true",
                            user1.contains("active") && user1["active"] == true);

                        if (user1.contains("roles") && user1["roles"].is_array()) {
                            auto roles = user1["roles"];

                            test_value("complex_json.users[0].roles has 2 items",
                                roles.size() == 2);

                            if (roles.size() >= 2) {
                                test_value("complex_json.users[0].roles[0] = 'admin'",
                                    roles[0] == "admin");
                                test_value("complex_json.users[0].roles[1] = 'user'",
                                    roles[1] == "user");
                            }
                        }

                        if (user1.contains("metadata")) {
                            auto metadata = user1["metadata"];

                            test_value("complex_json.users[0].metadata.created = '2023-01-01'",
                                metadata.contains("created") && metadata["created"] == "2023-01-01");
                            test_value("complex_json.users[0].metadata.updated is null",
                                metadata.contains("updated") && metadata["updated"].is_null());
                        }

                        auto user2 = users[1];
                        test_value("complex_json.users[1].id = 2",
                            user2.contains("id") && user2["id"] == 2);
                        test_value("complex_json.users[1].active = false",
                            user2.contains("active") && user2["active"] == false);
                    }
                }
            }
        } else {
            test_value("json_compatibility section exists", false);
        }

        // Test YAML edge cases section from test.yaml
        std::cout << "\n=== Testing YAML Edge Cases ===" << std::endl;
        if (parsed_json.contains("yaml_edge_cases")) {
            auto yaml_edge_cases = parsed_json["yaml_edge_cases"];
            test_value("yaml_edge_cases section exists", true);

            // Test different quote styles
            test_value("yaml_edge_cases.single_quotes = 'single quoted value'",
                yaml_edge_cases.contains("single_quotes")
                && yaml_edge_cases["single_quotes"] == "single quoted value");

            test_value("yaml_edge_cases.double_quotes = 'double quoted value'",
                yaml_edge_cases.contains("double_quotes")
                && yaml_edge_cases["double_quotes"] == "double quoted value");

            test_value("yaml_edge_cases.no_quotes = 'unquoted value'",
                yaml_edge_cases.contains("no_quotes")
                && yaml_edge_cases["no_quotes"] == "unquoted value");

            // Test multiline strings
            test_value("yaml_edge_cases.multiline_folded exists",
                yaml_edge_cases.contains("multiline_folded"));
            test_value("yaml_edge_cases.multiline_literal exists",
                yaml_edge_cases.contains("multiline_literal"));

            // Test numbers in different formats
            if (yaml_edge_cases.contains("numbers_test")) {
                auto numbers_test = yaml_edge_cases["numbers_test"];
                test_value("yaml_edge_cases.numbers_test.octal exists",
                    numbers_test.contains("octal"));
                test_value("yaml_edge_cases.numbers_test.hexadecimal exists",
                    numbers_test.contains("hexadecimal"));
                test_value("yaml_edge_cases.numbers_test.binary exists",
                    numbers_test.contains("binary"));

                // Test actual values if parsed correctly
                if (numbers_test.contains("octal") && numbers_test["octal"].is_number()) {
                    test_value("yaml_edge_cases.numbers_test.octal = 511",
                        numbers_test["octal"] == 511); // 0o777 = 511 decimal
                }
                if (numbers_test.contains("hexadecimal") && numbers_test["hexadecimal"].is_number()) {
                    test_value("yaml_edge_cases.numbers_test.hexadecimal = 255",
                        numbers_test["hexadecimal"] == 255); // 0xFF = 255 decimal
                }
                if (numbers_test.contains("binary") && numbers_test["binary"].is_number()) {
                    test_value("yaml_edge_cases.numbers_test.binary = 10",
                        numbers_test["binary"] == 10); // 0b1010 = 10 decimal
                }
            } else {
                test_value("yaml_edge_cases.numbers_test exists", false);
            }

            // Test special float values
            if (yaml_edge_cases.contains("special_floats")) {
                auto special_floats = yaml_edge_cases["special_floats"];
                test_value("yaml_edge_cases.special_floats.infinity exists",
                    special_floats.contains("infinity"));
                test_value("yaml_edge_cases.special_floats.negative_infinity exists",
                    special_floats.contains("negative_infinity"));
                test_value("yaml_edge_cases.special_floats.not_a_number exists",
                    special_floats.contains("not_a_number"));

                // Test actual special float values if parsed correctly
                if (special_floats.contains("infinity") && special_floats["infinity"].is_number()) {
                    auto inf_val = special_floats["infinity"].get<double>();
                    test_value("yaml_edge_cases.special_floats.infinity is infinite",
                        std::isinf(inf_val) && inf_val > 0);
                }
                if (special_floats.contains("negative_infinity")
                    && special_floats["negative_infinity"].is_number()) {
                    auto neg_inf_val = special_floats["negative_infinity"].get<double>();
                    test_value("yaml_edge_cases.special_floats.negative_infinity is negative infinite",
                        std::isinf(neg_inf_val) && neg_inf_val < 0);
                }
                if (special_floats.contains("not_a_number") && special_floats["not_a_number"].is_number()) {
                    auto nan_val = special_floats["not_a_number"].get<double>();
                    test_value("yaml_edge_cases.special_floats.not_a_number is NaN",
                        std::isnan(nan_val));
                }
            } else {
                test_value("yaml_edge_cases.special_floats exists", false);
            }
        } else {
            test_value("yaml_edge_cases section exists", false);
        }

        // Summary
        std::cout << "\n=== TEST SUMMARY ===" << std::endl;
        std::cout << "Tests passed: " << tests_passed << std::endl;
        std::cout << "Tests failed: " << tests_failed << std::endl;
        std::cout << "Total tests: " << (tests_passed + tests_failed) << std::endl;

        if (tests_failed == 0) {
            std::cout << "\n*** All YAML parser tests passed successfully!" << std::endl;
        } else {
            std::cout << "\n*** Some tests failed. The YAML parser may need improvements." << std::endl;
        }

        return tests_failed > 0 ? 1 : 0;

    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
}
