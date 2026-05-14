#pragma once

#include <cctype>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct Value {
  using Array = std::vector<Value>;
  using Object = std::unordered_map<std::string, Value>;
  std::variant<std::monostate, double, std::string, bool, Array, Object> item;
};

namespace JasonParser {

class parser {
public:
  parser(std::string_view source_code) : m_source_code(source_code) {}
  Value parse();

private:
  std::string_view m_source_code{};
  size_t m_current{};
  bool is_at_end() { return m_current >= m_source_code.size(); }
  char current() { return is_at_end() ? 0 : m_source_code[m_current]; }
  char advance() { return is_at_end() ? 0 : m_source_code[m_current++]; }
  void whitespace() {
    while (std::isspace(current()))
      advance();
  }
  void read(char c, std::string_view msg) {
    if (current() != c)
      throw std::runtime_error(std::string(msg));
    advance();
  }
  bool try_read(char c) { return current() == c ? (advance(), true) : false; }
  Value
  parse_value(); // EDGE CASE: split recursive parsing from top-level parse so
                 // only the outermost call rejects trailing garbage
  std::string parse_string();
  double parse_number();
  std::optional<Value> try_parse_keyword();
  Value::Array parse_array();
  Value::Object parse_object();
};

inline Value parser::parse() {
  Value v = parse_value();
  whitespace(); // EDGE CASE: allow whitespace between value and EOF, but reject
                // anything else
  if (!is_at_end()) // EDGE CASE: reject trailing garbage after top-level value
                    // (e.g. ["a"], or {} "extra")
    throw std::runtime_error("Unexpected trailing data");
  return v;
}

inline Value parser::parse_value() {
  whitespace(); // EDGE CASE: skip leading whitespace before every nested value
  switch (current()) {
  case '{':
    return Value{parse_object()};
  case '[':
    return Value{parse_array()};
  case '"':
    return Value{parse_string()};
  default:
    if (current() == '-' || std::isdigit(current()))
      return Value{parse_number()};
    if (auto kw = try_parse_keyword())
      return *kw;
    throw std::runtime_error("Unexpected character");
  }
}

inline std::string parser::parse_string() {
  read('"', "Expected an opening quote");
  std::string s;
  while (current() != '"') {
    if (is_at_end() || current() == '\n')
      throw std::runtime_error("Unterminated string");
    if (static_cast<unsigned char>(current()) <
        0x20) // EDGE CASE: JSON strings must not contain literal control
              // characters (U+0000–U+001F)
      throw std::runtime_error("Invalid control character in string");
    if (try_read('\\')) {
      char c = advance();
      switch (c) {
      case '"':
        s += '"';
        break;
      case '\\':
        s += '\\';
        break;
      case '/':
        s += '/';
        break;
      case 'b':
        s += '\b';
        break;
      case 'f':
        s += '\f';
        break;
      case 'n':
        s += '\n';
        break;
      case 'r':
        s += '\r';
        break;
      case 't':
        s += '\t';
        break;
      case 'u': { // EDGE CASE: handle \uXXXX escapes including surrogate pairs
                  // for code points above U+FFFF
        auto hex_digit = [&]() {
          char hc = advance();
          if (hc >= '0' && hc <= '9')
            return hc - '0';
          if (hc >= 'a' && hc <= 'f')
            return hc - 'a' + 10;
          if (hc >= 'A' && hc <= 'F')
            return hc - 'A' + 10;
          throw std::runtime_error("Invalid hex digit");
        };
        int cp = (hex_digit() << 12) | (hex_digit() << 8) | (hex_digit() << 4) |
                 hex_digit();
        if (cp >= 0xD800 && cp <= 0xDBFF) {
          if (!try_read('\\') || advance() != 'u')
            throw std::runtime_error("Expected low surrogate");
          int low = (hex_digit() << 12) | (hex_digit() << 8) |
                    (hex_digit() << 4) | hex_digit();
          if (low < 0xDC00 || low > 0xDFFF)
            throw std::runtime_error("Invalid low surrogate");
          cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
        }
        if (cp < 0x80) {
          s += static_cast<char>(cp);
        } else if (cp < 0x800) {
          s += static_cast<char>(0xC0 | (cp >> 6));
          s += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
          s += static_cast<char>(0xE0 | (cp >> 12));
          s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
          s += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
          s += static_cast<char>(0xF0 | (cp >> 18));
          s += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
          s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
          s += static_cast<char>(0x80 | (cp & 0x3F));
        }
        break;
      }
      default:
        throw std::runtime_error("Invalid escape sequence");
      }
    } else {
      s += advance();
    }
  }
  read('"', "Expected a closing quote");
  return s;
}

inline double parser::parse_number() {
  size_t start = m_current;
  try_read('-');
  if (!std::isdigit(
          current())) // EDGE CASE: reject "-" or "e" alone as a number
    throw std::runtime_error("Invalid number");
  while (std::isdigit(current()))
    advance();
  size_t digit_start = (m_source_code[start] == '-') ? start + 1 : start;
  if (m_current > digit_start + 1 &&
      m_source_code[digit_start] ==
          '0') // EDGE CASE: reject leading zeros like "013" (JSON forbids them)
    throw std::runtime_error("Leading zero");
  if (try_read('.')) {
    if (!std::isdigit(
            current())) // EDGE CASE: reject "1." with no fraction digits
      throw std::runtime_error("Missing fraction digits");
    while (std::isdigit(current()))
      advance();
  }
  if (try_read('e') || try_read('E')) {
    try_read('+') || try_read('-');
    if (!std::isdigit(current())) // EDGE CASE: reject "0e" or "0e+" where
                                  // std::stod silently parses just the mantissa
      throw std::runtime_error("Missing exponent digits");
    while (std::isdigit(current()))
      advance();
  }
  std::string num_str(m_source_code.substr(start, m_current - start));
  size_t idx = 0;
  double val = std::stod(num_str, &idx);
  if (idx != num_str.size()) // EDGE CASE: std::stod may stop early (e.g. "0e"
                             // -> "0"); ensure it consumed the entire token
    throw std::runtime_error("Invalid number");
  return val;
}

inline std::optional<Value> parser::try_parse_keyword() {
  if (m_source_code.compare(m_current, 4, "true") == 0) {
    m_current += 4;
    return Value{true};
  }
  if (m_source_code.compare(m_current, 5, "false") == 0) {
    m_current += 5;
    return Value{false};
  }
  if (m_source_code.compare(m_current, 4, "null") == 0) {
    m_current += 4;
    return Value{std::monostate{}};
  }
  return std::nullopt;
}

inline Value::Array parser::parse_array() {
  Value::Array a;
  read('[', "Expected an opening bracket");
  whitespace(); // EDGE CASE: allow whitespace before empty-array close (e.g. "[
                // ]")
  if (try_read(']'))
    return a;
  do {
    whitespace();
    a.push_back(parse_value()); // EDGE CASE: use parse_value for nested values
                                // so parse() only runs at the top level
    whitespace();
  } while (try_read(','));
  read(']', "Expected a closing bracket");
  return a;
}

inline Value::Object parser::parse_object() {
  Value::Object o;
  read('{', "Expected an opening brace");
  whitespace(); // EDGE CASE: allow whitespace before empty-object close (e.g.
                // "{  }")
  if (try_read('}'))
    return o;
  do {
    whitespace();
    auto k = parse_string();
    whitespace();
    read(':', "Expected a colon");
    whitespace();
    o.emplace(k, parse_value()); // EDGE CASE: use parse_value for nested values
                                 // so parse() only runs at the top level
    whitespace();
  } while (try_read(','));
  read('}', "Expected a closing brace");
  return o;
}

} // namespace JasonParser
