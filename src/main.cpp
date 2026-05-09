#include <fstream>
#include <iostream>
#include <string>

#include "jason.h"

int main() {
  std::fstream file{"tests/example.json"};
  std::string content{};
  std::string line{};
  while (std::getline(file, line)) {
    content += line + "\n";
  }
  std::cout << content;

  JasonParser::parser parser{content};
  parser.parse();
  return 0;
}
