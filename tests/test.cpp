#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "jason.h"

int main() {
  std::ifstream file{"tests/example.json"};
  std::stringstream buffer;
  buffer << file.rdbuf();

  std::string content{buffer.str()};
  JasonParser::parser parser{content};
  auto value = parser.parse();

  bool passed = true;

  if (auto *obj = std::get_if<Value::Object>(&value.item)) {
    if (auto it = obj->find("string"); it != obj->end()) {
      if (auto *str = std::get_if<std::string>(&it->second.item)) {
        if (*str != "hello") {
          std::cerr << "FAIL: string != hello" << std::endl;
          passed = false;
        }
      }
    }
    if (auto it = obj->find("integer"); it != obj->end()) {
      if (auto *num = std::get_if<double>(&it->second.item)) {
        if (*num != 42) {
          std::cerr << "FAIL: integer != 42" << std::endl;
          passed = false;
        }
      }
    }
    if (auto it = obj->find("boolean"); it != obj->end()) {
      if (auto *b = std::get_if<bool>(&it->second.item)) {
        if (*b != true) {
          std::cerr << "FAIL: boolean != true" << std::endl;
          passed = false;
        }
      }
    }
    if (auto it = obj->find("null"); it != obj->end()) {
      if (!std::holds_alternative<std::monostate>(it->second.item)) {
        std::cerr << "FAIL: null is not null" << std::endl;
        passed = false;
      }
    }
  } else {
    std::cerr << "FAIL: root is not an object" << std::endl;
    passed = false;
  }

  if (passed) {
    std::cout << "ALLES GUT" << std::endl;
  }

  return passed ? 0 : 1;
}
