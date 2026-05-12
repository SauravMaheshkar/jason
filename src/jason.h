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
  std::string parse_string();
  double parse_number();
  std::optional<Value> try_parse_keyword();
  Value::Array parse_array();
  Value::Object parse_object();
};

inline Value parser::parse() {
  whitespace();
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
    throw std::runtime_error("Unexpected character: ");
  }
}

inline std::string parser::parse_string() {
  read('"', "Expected an opening quote");
  std::string s;
  while (current() != '"') {
    if (is_at_end() || current() == '\n')
      throw std::runtime_error("Unterminated string");
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
  while (std::isdigit(current()))
    advance();
  if (try_read('.'))
    while (std::isdigit(current()))
      advance();
  if (try_read('e') || try_read('E')) {
    try_read('+') || try_read('-');
    while (std::isdigit(current()))
      advance();
  }
  return std::stod(std::string(m_source_code.substr(start, m_current - start)));
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
  if (try_read(']'))
    return a;
  do {
    whitespace();
    a.push_back(parse());
    whitespace();
  } while (try_read(','));
  read(']', "Expected a closing bracket");
  return a;
}

inline Value::Object parser::parse_object() {
  Value::Object o;
  read('{', "Expected an opening brace");
  if (try_read('}'))
    return o;
  do {
    whitespace();
    auto k = parse_string();
    whitespace();
    read(':', "Expected a colon");
    whitespace();
    o.emplace(k, parse());
    whitespace();
  } while (try_read(','));
  read('}', "Expected a closing brace");
  return o;
}

} // namespace JasonParser
