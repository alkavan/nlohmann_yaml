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

#ifndef NLOHMANN_YAML_HPP
#define NLOHMANN_YAML_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <limits>

namespace nlohmann {
    /**
     * YAML parsing class providing functionality for parsing YAML inputs, extracting
     * structures, managing indentation, and handling embedded JSON blocks.
     */
    class yaml_parser {
        private:
        std::vector<std::string> lines;
        size_t current_line = 0;

        /**
         * Preprocesses the input stream by removing comments, trimming trailing whitespace,
         * and storing the resultant lines while preserving the original line structure.
         *
         * @param input The input stream to preprocess. Typically, it contains raw YAML content.
         */
        void preprocess_input(std::istream& input) {
            std::string line;
            while (std::getline(input, line)) {
                // Remove comments
                if (const size_t comment_pos = line.find('#'); comment_pos != std::string::npos) {
                    line = line.substr(0, comment_pos);
                }
                // Remove trailing whitespace (handle whitespace-only lines safely)
                if (!line.empty()) {
                    if (const auto last = line.find_last_not_of(" \t\r\n"); last != std::string::npos) {
                        line.erase(last + 1);
                    } else {
                        line.clear();
                    }
                }

                // Keep all lines, including empty ones, to maintain the line structure
                lines.push_back(line);
            }
        }

        /**
         * Calculates the indentation level of a given line based on spaces and tabs.
         *
         * @param line The string for which the indentation level needs to be calculated.
         * @return The total indentation count, treating spaces as 1 unit and tabs as 2 units by default.
         */
        static int get_indent(const std::string& line) {
            int indent = 0;
            for (const char i : line) {
                if (i == ' ') {
                    indent++;
                } else if (i == '\t') {
                    indent += 2;  // Treat tab as 2 spaces (adjust to 4 or 8 if preferred based on convention)
                } else {
                    break;
                }
            }
            return indent;
        }

        /**
         * Determines the next line's indentation level that is greater than the parent indentation,
         * starting from a specified line index. Skips empty lines and stops search at the first
         * line with an indentation level that does not match the condition.
         *
         * @param start_line The index of the line to start checking from.
         * @param parent_indent The indentation level of the parent line used as a reference.
         * @return The indentation level of the next suitable line if it exists; otherwise, returns -1.
         */
        [[nodiscard]] int get_next_sub_indent(const size_t start_line, const int parent_indent) const {
            size_t peek = start_line;
            while (peek < lines.size()) {
                const std::string& p_line = lines[peek];

                if (p_line.empty()) {
                    ++peek;
                    continue;
                }

                if (const int p_indent = get_indent(p_line); p_indent > parent_indent) {
                    return p_indent;
                }

                return -1;
            }
            return -1;
        }

        /**
         * Parses a JSON array from a given string and returns the corresponding JSON object.
         * The input string must represent a valid JSON array syntax.
         *
         * @param str The string containing the JSON array to parse.
         * @return A JSON object representing the parsed array.
         * @throws std::runtime_error If the input string is not a valid JSON array.
         */
        static json parse_json_array(const std::string& str) {
            try {
                return json::parse(str);
            } catch (...) {
                throw std::runtime_error("Invalid JSON array syntax: " + str);
            }
        }

        /**
         * Parses a JSON object from a given string and returns the corresponding JSON representation.
         * The input string must represent a valid JSON object syntax.
         *
         * @param str The string containing the JSON object to parse.
         * @return A JSON object representing the parsed input string.
         * @throws std::runtime_error If the input string is not a valid JSON object.
         */
        static json parse_json_object(const std::string& str) {
            try {
                return json::parse(str);
            } catch (...) {
                throw std::runtime_error("Invalid JSON object syntax: " + str);
            }
        }

        /**
         * Determines whether the provided string represents a valid JSON array.
         * A valid JSON array is identified by being non-empty, trimmed of leading
         * and trailing whitespace, and starting with '[' and ending with ']'.
         *
         * @param str The string to check for JSON array syntax.
         * @return True if the string represents a JSON array, otherwise false.
         */
        static bool is_json_array(const std::string& str) {
            std::string trimmed = str;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
            return !trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']';
        }

        /**
         * Determines whether the provided string represents a valid JSON object.
         * A valid JSON object is identified by being non-empty, trimmed of leading
         * and trailing whitespace, and starting with '{' and ending with '}'.
         *
         * @param str The string to check for JSON object syntax.
         * @return True if the string represents a JSON object, otherwise false.
         */
        static bool is_json_object(const std::string& str) {
            std::string trimmed = str;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            trimmed.erase(trimmed.find_last_not_of(" \t") + 1);
            return !trimmed.empty() && trimmed.front() == '{' && trimmed.back() == '}';
        }

        /**
         * Determines whether the given string starts with a JSON token
         * after ignoring leading whitespace.
         *
         * @param str The input string to analyze.
         * @return True if the string starts with a '[' or '{' character after
         *         whitespace is trimmed; otherwise, false.
         */
        static bool starts_with_json_token(const std::string& str) {
            const size_t i = str.find_first_not_of(" \t");
            if (i == std::string::npos) return false;
            const char c = str[i];
            return c == '[' || c == '{';
        }

        /**
         * Attempts to collect a contiguous JSON block from the current position in the input lines,
         * starting at a specified indentation level.
         *
         * A valid JSON block must begin with either '{' or '[' and adhere to a proper JSON structure,
         * including balanced brackets and braces. The process respects string literals and escape
         * characters to avoid misinterpreting JSON content.
         *
         * @param current_indent The expected indentation level for the JSON block to start.
         * @param out_json A reference to a string where the collected JSON block will be stored if successfully found.
         * @return A boolean indicating whether a valid JSON block was successfully collected.
         *         Returns true if a valid JSON block is found, false otherwise.
         */
        bool try_collect_json_block(const int current_indent, std::string& out_json) {
            const size_t saved_line = current_line;

            size_t i = current_line;
            bool started = false;

            int curly = 0;
            int square = 0;
            bool in_string = false;
            char quote_char = '\0';
            bool escape = false;

            std::string buffer;

            while (i < lines.size()) {
                const std::string& raw = lines[i];

                // Skip empty lines before we start
                if (!started && raw.empty()) {
                    i++;
                    continue;
                }

                const int indent = get_indent(raw);

                // If we haven't started yet, ensure this line is at the expected indent and starts with { or [
                if (!started) {
                    if (indent != current_indent) {
                        break;
                    }
                    std::string content = raw.substr(indent);
                    // Trim leading spaces for start check
                    const size_t s = content.find_first_not_of(" \t");
                    if (s == std::string::npos) {
                        // nothing meaningful on this line
                        i++;
                        continue;
                    }
                    if (const char first = content[s]; first != '{' && first != '[') {
                        // Not a JSON block start
                        break;
                    }
                    started = true;
                }

                // Once started, lines with indent less than current_indent terminate unless we already balanced
                if (started && indent < current_indent) {
                    break;
                }

                const std::string content = raw.substr(indent);

                // Append content to buffer (newline separates lines; JSON allows whitespace)
                if (!buffer.empty()) {
                    buffer.push_back('\n');
                }
                buffer.append(content);

                // Scan content to update bracket/brace counters while respecting strings
                for (const char c : content) {
                    if (in_string) {
                        if (escape) {
                            escape = false;
                        } else if (c == '\\') {
                            escape = true;
                        } else if (c == quote_char) {
                            in_string = false;
                        }
                    } else {
                        if (c == '"' || c == '\'') {
                            in_string = true;
                            quote_char = c;
                        } else if (c == '{') {
                            ++curly;
                        } else if (c == '}') {
                            --curly;
                        } else if (c == '[') {
                            ++square;
                        } else if (c == ']') {
                            --square;
                        }
                    }
                }

                ++i;

                if (started && curly <= 0 && square <= 0) {
                    // Completed a JSON value
                    current_line = i;
                    out_json = buffer;
                    return true;
                }
            }

            // Not a valid/complete JSON block – restore and signal failure
            current_line = saved_line;
            return false;
        }

        /**
         * Parses a scalar value from a string and converts it to the appropriate JSON-compatible type.
         * Handles various data formats such as strings, numbers, booleans, nulls, and special YAML values.
         * Also, processes quoted strings and escaped characters.
         *
         * @param value The input string containing the scalar value to parse.
         * @return A JSON array representing the parsed sequence.
         */
        static json parse_scalar(const std::string& value) {
            std::string val = value;

            // Remove leading/trailing whitespace
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t") + 1);

            // Check for JSON array syntax
            if (is_json_array(val)) {
                return parse_json_array(val);
            }

            // Check for JSON object syntax
            if (is_json_object(val)) {
                return parse_json_object(val);
            }

            // Remove quotes if present
            if (!val.empty() && ((val.front() == '"' && val.back() == '"') ||
                                (val.front() == '\'' && val.back() == '\''))) {
                val = val.substr(1, val.size() - 2);
                // Handle escaped characters in quoted strings
                std::string result;
                for (size_t i = 0; i < val.size(); ++i) {
                    if (val[i] == '\\' && i + 1 < val.size()) {
                        switch (val[i + 1]) {
                            case 'n': result += '\n'; break;
                            case 't': result += '\t'; break;
                            case 'r': result += '\r'; break;
                            case '\\': result += '\\'; break;
                            case '"': result += '"'; break;
                            case '\'': result += '\''; break;
                            default: result += val[i + 1]; break;
                        }
                        ++i; // skip the escaped character
                    } else {
                        result += val[i];
                    }
                }
                return result;
            }

            // Handle special YAML values
            if (val == "null" || val == "~" || val == "Null" || val == "NULL") return nullptr;
            if (val == "true" || val == "True" || val == "TRUE") return true;
            if (val == "false" || val == "False" || val == "FALSE") return false;

            // Handle special float values
            if (val == ".inf" || val == ".Inf" || val == ".INF" || val == "+.inf") {
                return std::numeric_limits<double>::infinity();
            }

            if (val == "-.inf" || val == "-.Inf" || val == "-.INF") {
                return -std::numeric_limits<double>::infinity();
            }

            if (val == ".nan" || val == ".NaN" || val == ".NAN") {
                return std::numeric_limits<double>::quiet_NaN();
            }

            // Try parsing as different number formats
            try {
                // Handle different number bases
                if (val.size() > 2 && val[0] == '0') {
                    if (val[1] == 'x' || val[1] == 'X') {
                        // Hexadecimal
                        return std::stoi(val, nullptr, 16);
                    } else if (val[1] == 'o' || val[1] == 'O') {
                        // Octal
                        return std::stoi(val.substr(2), nullptr, 8);
                    } else if (val[1] == 'b' || val[1] == 'B') {
                        // Binary
                        return std::stoi(val.substr(2), nullptr, 2);
                    }
                }

                // Handle scientific notation and regular numbers
                if (val.find('.') != std::string::npos
                    || val.find('e') != std::string::npos
                    || val.find('E') != std::string::npos) {
                    return std::stod(val);
                }

                // Handle negative numbers
                if (val[0] == '-' || val[0] == '+') {
                    if (val.find('.') != std::string::npos
                        || val.find('e') != std::string::npos
                        || val.find('E') != std::string::npos) {
                        return std::stod(val);
                    } else {
                        return std::stoll(val);
                    }
                }

                return std::stoll(val);
            } catch (...) {
                return val; // Return as string
            }
        }

        /**
         * Parses a YAML-style sequence from the current position in the input lines, constructing a JSON array
         * with the appropriate structure based on indentation and content rules.
         *
         * @param current_indent The current level of indentation in the input, used to determine sequence boundaries.
         * @return A JSON array representing the parsed sequence.
         * @throws std::runtime_error If syntax issues or unexpected input structures are encountered during parsing.
         */
        json parse_sequence(const int current_indent) {
            json array = json::array();

            while (current_line < lines.size()) {
                const std::string& line = lines[current_line];

                // Skip empty lines
                if (line.empty()) {
                    current_line++;
                    continue;
                }

                const int line_indent = get_indent(line);

                // If indentation is less than the current level, we're done
                if (line_indent < current_indent) {
                    break;
                }

                // If indentation doesn't match, break
                if (line_indent != current_indent) {
                    break;
                }

                // Check if this is a sequence item
                if (line[line_indent] != '-') {
                    break;
                }

                current_line++;

                // Extract the value after the dash
                std::string value = line.substr(line_indent + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                if (value.empty()) {
                    // Complex value on next line(s)
                    int sub_indent = get_next_sub_indent(current_line, current_indent);
                    if (sub_indent == -1) {
                        throw std::runtime_error("Expected indented block for sequence item at line "
                            + std::to_string(current_line - 1));
                    }
                    json sub = parse_value(sub_indent);
                    if (sub.is_null()) {
                        throw std::runtime_error("Failed to parse block for sequence item at line "
                            + std::to_string(current_line - 1));
                    }
                    array.push_back(sub);
                } else if (!value.empty() && value[0] == '-') {
                    // Inline nested sequence - handle specially
                    json nested_array = json::array();

                    // Parse the current line as nested sequence items
                    std::string remaining = value;
                    while (!remaining.empty() && remaining[0] == '-') {
                        remaining = remaining.substr(1); // Remove the dash
                        remaining.erase(0, remaining.find_first_not_of(" \t"));

                        // Find the next dash or end of line
                        size_t next_dash = remaining.find(" -");
                        std::string item_value;
                        if (next_dash != std::string::npos) {
                            item_value = remaining.substr(0, next_dash);
                            remaining = remaining.substr(next_dash + 1);
                            remaining.erase(0, remaining.find_first_not_of(" \t"));
                        } else {
                            item_value = remaining;
                            remaining.clear();
                        }

                        if (!item_value.empty()) {
                            nested_array.push_back(parse_scalar(item_value));
                        }
                    }

                    // Now check for continuation lines at higher indentation
                    int sub_indent = -1;
                    while (current_line < lines.size()) {
                        const std::string& next_line = lines[current_line];
                        if (next_line.empty()) {
                            current_line++;
                            continue;
                        }

                        const int next_indent = get_indent(next_line);
                        if (next_indent <= current_indent) {
                            break; // End of this nested sequence
                        }

                        if (next_line[next_indent] == '-') {
                            if (sub_indent == -1) {
                                sub_indent = next_indent;
                            } else if (next_indent != sub_indent) {
                                throw std::runtime_error(
                                    "Inconsistent indentation in nested sequence continuation at line "
                                    + std::to_string(current_line));
                            }
                            current_line++;
                            std::string next_value = next_line.substr(next_indent + 1);
                            next_value.erase(0, next_value.find_first_not_of(" \t"));
                            nested_array.push_back(parse_scalar(next_value));
                        } else {
                            break;
                        }
                    }

                    array.push_back(nested_array);
                } else if (value.find(':') != std::string::npos) {
                    // Inline mapping
                    json obj = json::object();

                    // Parse the first key-value pair from the current line
                    size_t colon_pos = value.find(':');
                    std::string key = value.substr(0, colon_pos);
                    key.erase(key.find_last_not_of(" \t") + 1);

                    std::string val = value.substr(colon_pos + 1);
                    val.erase(0, val.find_first_not_of(" \t"));

                    if (val.empty()) {
                        int sub_indent = get_next_sub_indent(current_line, current_indent);
                        if (sub_indent == -1) {
                            throw std::runtime_error("Expected indented block for key '" + key
                                + "' at line " + std::to_string(current_line - 1));
                        }
                        json sub = parse_value(sub_indent);
                        if (sub.is_null()) {
                            throw std::runtime_error("Failed to parse block for key '" + key
                                + "' at line " + std::to_string(current_line - 1));
                        }
                        obj[key] = sub;
                    } else {
                        obj[key] = parse_scalar(val);
                    }

                    // Now check for additional key-value pairs at a consistent higher indentation
                    int key_indent = -1;
                    while (current_line < lines.size()) {
                        const std::string& next_line = lines[current_line];
                        if (next_line.empty()) {
                            current_line++;
                            continue;
                        }

                        const int next_indent = get_indent(next_line);

                        // If indentation is less than or equal to sequence item level, we're done
                        if (next_indent <= current_indent) {
                            break;
                        }

                        // Must have a colon to be a mapping entry
                        size_t next_colon_pos = next_line.find(':');
                        if (next_colon_pos == std::string::npos) {
                            break;
                        }

                        // Set or check consistent key indentation
                        if (key_indent == -1) {
                            key_indent = next_indent;
                        } else if (next_indent != key_indent) {
                            break;
                        }

                        current_line++;

                        std::string next_key = next_line.substr(next_indent, next_colon_pos - next_indent);
                        next_key.erase(next_key.find_last_not_of(" \t") + 1);

                        std::string next_val = next_line.substr(next_colon_pos + 1);
                        next_val.erase(0, next_val.find_first_not_of(" \t"));

                        if (next_val.empty()) {
                            int next_sub_indent = get_next_sub_indent(current_line, key_indent);
                            if (next_sub_indent == -1) {
                                throw std::runtime_error("Expected indented block for key '" + next_key
                                    + "' at line " + std::to_string(current_line - 1));
                            }
                            json next_sub = parse_value(next_sub_indent);
                            if (next_sub.is_null()) {
                                throw std::runtime_error("Failed to parse block for key '" + next_key
                                    + "' at line " + std::to_string(current_line - 1));
                            }
                            obj[next_key] = next_sub;
                        } else {
                            obj[next_key] = parse_scalar(next_val);
                        }
                    }

                    array.push_back(obj);
                } else {
                    // Simple scalar value (including JSON arrays and objects)
                    array.push_back(parse_scalar(value));
                }
            }

            return array;
        }

        /**
         * Parses a YAML-style mapping structure from the current line and returns it
         * as a JSON object. The method verifies indentation consistency, extracts keys
         * and values, handles nested mappings, and processes scalar values.
         *
         * @param current_indent The current indentation level to compare against the lines
         *                       being parsed. Determines the scope of the mapping.
         * @return A JSON object constructed from the parsed mapping structure.
         *         Keys represent YAML mapping keys, and values represent their corresponding parsed values.
         * @throws std::runtime_error If the expected structure (e.g., indented block for a key)
         *                            is missing, or parsing fails for nested blocks.
         */
        json parse_mapping(const int current_indent) {
            json object = json::object();

            while (current_line < lines.size()) {
                const std::string& line = lines[current_line];

                // Skip empty lines
                if (line.empty()) {
                    current_line++;
                    continue;
                }

                const int line_indent = get_indent(line);

                // If indentation is less than the current level, we're done
                if (line_indent < current_indent) {
                    break;
                }

                // If indentation doesn't match the current level, break
                if (line_indent != current_indent) {
                    break;
                }

                // Look for key-value separator
                const size_t colon_pos = line.find(':');
                if (colon_pos == std::string::npos) {
                    break; // Not a mapping line
                }

                current_line++;

                // Extract key and value
                std::string key = line.substr(line_indent, colon_pos - line_indent);
                key.erase(key.find_last_not_of(" \t") + 1);

                std::string value = line.substr(colon_pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                if (value.empty()) {
                    // Complex value on next line(s)
                    const int sub_indent = get_next_sub_indent(current_line, current_indent);
                    if (sub_indent == -1) {
                        throw std::runtime_error("Expected indented block for key '" + key
                            + "' at line " + std::to_string(current_line - 1));
                    }
                    json sub = parse_value(sub_indent);
                    if (sub.is_null()) {
                        throw std::runtime_error("Failed to parse block for key '" + key
                            + "' at line " + std::to_string(current_line - 1));
                    }
                    object[key] = sub;
                } else {
                    // Simple scalar value (including JSON arrays and objects)
                    object[key] = parse_scalar(value);
                }
            }

            return object;
        }

        /**
         * Parses a single value from the current YAML lines using the provided indentation level.
         * Determines the type of the value (scalar, sequence, mapping, or JSON block) based on
         * the content and indentation rules.
         *
         * @param current_indent The expected indentation level for parsing the current value.
         *                        Lines with greater indentation are ignored, and lines with
         *                        lesser indentation indicate the end of the current structure.
         * @return A JSON representation of the parsed value. Returns `nullptr` if no valid value
         *         can be parsed or if the operation reaches the end of the input lines.
         */
        json parse_value(const int current_indent) {
            while (current_line < lines.size()) {
                const std::string& line = lines[current_line];

                // Skip empty lines
                if (line.empty()) {
                    current_line++;
                    continue;
                }

                const int line_indent = get_indent(line);

                // If indentation is less than expected, return null
                if (line_indent < current_indent) {
                    return nullptr;
                }

                // If indentation matches, determine the type
                if (line_indent == current_indent) {
                    const std::string at_level = line.substr(line_indent);

                    // If this line starts with a JSON token, try to parse a (potentially multi-line) JSON block
                    if (starts_with_json_token(at_level)) {
                        const size_t saved = current_line;
                        if (std::string json_text; try_collect_json_block(current_indent, json_text)) {
                            try {
                                return json::parse(json_text);
                            } catch (...) {
                                // If parsing fails, revert and fall through to other handlers
                                current_line = saved;
                            }
                        }
                    }

                    if (line[line_indent] == '-') {
                        return parse_sequence(current_indent);
                    } else if (line.find(':') != std::string::npos) {
                        return parse_mapping(current_indent);
                    } else {
                        current_line++;
                        return parse_scalar(at_level);
                    }
                } else {
                    // Skip lines with greater indentation until we find our level
                    current_line++;
                    continue;
                }
            }

            return nullptr;
        }

    public:
        /**
         * Constructs a YAML parser object initialized with an input stream. The input
         * stream is preprocessed to prepare the parser for analyzing the YAML content.
         *
         * @param is An input stream containing the raw YAML data to be parsed.
         */
        explicit yaml_parser(std::istream& is) {
            preprocess_input(is);
        }

        /**
         * Parses a YAML-like document into a JSON object. It processes the input lines, identifying
         * and handling mappings, sequences, and scalar values. This method expects a line-based
         * representation of a YAML document and assumes correct formatting.
         *
         * @return A JSON object representing the parsed structure of the input document.
         *         This includes mappings, sequences, scalar values, and nested structures.
         *         Throws an exception if the input structure is invalid or unprocessable.
         */
        json parse() {
            json root = json::object();
            current_line = 0;

            while (current_line < lines.size()) {
                const std::string& line = lines[current_line];

                // Skip empty lines
                if (line.empty()) {
                    current_line++;
                    continue;
                }

                const int line_indent = get_indent(line);

                // Check if this is a sequence at the root level
                if (line[0] == '-') {
                    if (root.empty()) {
                        return parse_sequence(0);
                    } else {
                        throw std::runtime_error("Cannot mix sequences and mappings at root level");
                    }
                }

                // Look for mapping
                const size_t colon_pos = line.find(':');
                if (colon_pos == std::string::npos) {
                    current_line++;
                    continue; // Skip lines that aren't key-value pairs
                }

                // Extract key and value
                std::string key = line.substr(line_indent, colon_pos - line_indent);
                key.erase(key.find_last_not_of(" \t") + 1);

                std::string value = line.substr(colon_pos + 1);
                value.erase(0, value.find_first_not_of(" \t"));

                current_line++;

                if (value.empty()) {
                    // Complex value on next line(s)
                    const int sub_indent = get_next_sub_indent(current_line, line_indent);
                    if (sub_indent == -1) {
                        throw std::runtime_error("Expected indented block for key '" + key
                            + "' at line " + std::to_string(current_line - 1));
                    }

                    json sub = parse_value(sub_indent);
                    if (sub.is_null()) {
                        throw std::runtime_error("Failed to parse block for key '" + key
                            + "' at line " + std::to_string(current_line - 1));
                    }

                    root[key] = sub;
                } else {
                    // Simple scalar value (including JSON arrays and objects)
                    root[key] = parse_scalar(value);
                }
            }

            return root;
        }
    };

    /**
     * Parses a YAML input stream and converts it to a JSON object.
     *
     * @param input The input stream containing YAML data to be parsed.
     * @return A JSON object representing the parsed data from the YAML input.
     */
    inline json parse_yaml(std::istream& input) {
        yaml_parser parser(input);
        return parser.parse();
    }

    /**
     * Parses a YAML string and converts it to a JSON object.
     *
     * @param input The input string containing YAML data to be parsed.
     * @return A JSON object representing the parsed data from the YAML string.
     */
    inline json parse_yaml(const std::string& input) {
        std::istringstream iss(input);
        return parse_yaml(iss);
    }

} // namespace nlohmann

#endif // NLOHMANN_YAML_HPP