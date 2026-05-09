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

namespace JasonValue {
struct Value {
  using Array = std::vector<Value>;
  using Object = std::unordered_map<std::string, Value>;
  std::variant<std::monostate, double, std::string, bool, Array, Object> item;
};
} // namespace JasonValue

using Value = JasonValue::Value;

namespace JasonParser {
class parser {
public:
  parser(std::string_view source_code);
  Value parse();

private:
  std::string_view m_source_code{};
  size_t m_current{};
  bool is_at_end();
  char current();
  char advance();
  void whitespace();
  void read(char c, std::string_view msg);
  bool try_read(char c);
  std::string parse_string();
  double parse_number();
  std::optional<Value> try_parse_keyword();
  Value::Array parse_array();
  Value::Object parse_object();
};

inline parser::parser(std::string_view source_code)
    : m_source_code{source_code} {}

inline Value parser::parse() {
  whitespace();
  char c{current()};
  switch (c) {
  case '{':
    return Value{parse_object()};
  case '[':
    return Value{parse_array()};
  case '"':
    return Value{parse_string()};
  default:
    if (std::isdigit(c))
      return Value{parse_number()};
    else if (auto kw{try_parse_keyword()}; kw.has_value())
      return *kw;
    else
      throw std::runtime_error("Unexpected character: ");
  }
  return Value{std::monostate{}};
}

inline bool parser::is_at_end() { return m_current >= m_source_code.size(); }

inline char parser::current() {
  return is_at_end() ? 0 : m_source_code[m_current];
}

inline char parser::advance() {
  return is_at_end() ? 0 : m_source_code[m_current++];
}

inline void parser::whitespace() {
  while (std::isspace(current()))
    advance();
}

inline void parser::read(char c, std::string_view msg) {
  if (current() != c)
    throw std::runtime_error{std::string(msg)};
  advance();
}

inline bool parser::try_read(char c) {
  return current() == c ? (advance(), true) : false;
}

inline std::string parser::parse_string() {
  read('"', "Expected an opening quote");
  size_t start{m_current};
  while (current() != '"') {
    advance();
    if (is_at_end() || current() == '\n')
      break;
  }
  size_t end{m_current};
  read('"', "Expected a closing quote");
  return std::string{m_source_code.substr(start, end - start)};
}

inline double parser::parse_number() {
  size_t start{m_current};
  while (std::isdigit(current()) && !is_at_end())
    advance();
  if (try_read('.'))
    while (std::isdigit(current()) && !is_at_end())
      advance();
  size_t end{m_current};
  return std::stod(std::string{m_source_code.substr(start, end - start)});
}

inline std::optional<Value> parser::try_parse_keyword() {
  std::unordered_map<std::string, Value> m{{"true", Value{true}},
                                           {"false", Value{false}},
                                           {"null", Value{std::monostate{}}}};
  for (const auto &kv : m) {
    auto end{m_current + kv.first.length()};
    if (end >= m_source_code.size())
      continue;
    if (m_source_code.substr(m_current, kv.first.length()) == kv.first) {
      m_current += kv.first.length();
      return kv.second;
    }
  }
  return std::nullopt;
}

inline Value::Array parser::parse_array() {
  Value::Array a{};
  read('[', "Expected an opening bracket");
  while (!try_read(']')) {
    whitespace();
    a.push_back(parse());
    whitespace();
    if (try_read(']'))
      break;
    read(',', "Expected a comma or closing bracket");
  }
  return a;
}

inline Value::Object parser::parse_object() {
  Value::Object o{};
  read('{', "Expected an opening brace");
  while (!try_read('}')) {
    whitespace();
    auto k{parse_string()};
    whitespace();
    read(':', "Expected a colon");
    whitespace();
    o.insert({k, parse()});
    whitespace();
    if (try_read('}'))
      break;
    read(',', "Expected a comma or closing brace");
  }
  return o;
}
} // namespace JasonParser
