#include "jason/jason.h"
#include "../test.h"

#include <type_traits>
#include <variant>

using namespace JasonValue;
using namespace JasonParser;

static void GenStat(Stat &stat, const Value &v) {
  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          stat.nullCount++;
        } else if constexpr (std::is_same_v<T, bool>) {
          if (arg)
            stat.trueCount++;
          else
            stat.falseCount++;
        } else if constexpr (std::is_same_v<T, double>) {
          stat.numberCount++;
        } else if constexpr (std::is_same_v<T, std::string>) {
          stat.stringCount++;
          stat.stringLength += arg.size();
        } else if constexpr (std::is_same_v<T, Value::Array>) {
          for (const auto &element : arg)
            GenStat(stat, element);
          stat.arrayCount++;
          stat.elementCount += arg.size();
        } else if constexpr (std::is_same_v<T, Value::Object>) {
          for (const auto &kv : arg) {
            stat.stringLength += kv.first.size();
            GenStat(stat, kv.second);
          }
          stat.objectCount++;
          stat.memberCount += arg.size();
          stat.stringCount += arg.size(); // keys are strings
        }
      },
      v.item);
}

class JasonParseResult : public ParseResultBase {
public:
  Value root;
};

class JasonTest : public TestBase {
public:
#if TEST_INFO
  virtual const char *GetName() const { return "jason (C++17)"; }
  virtual const char *GetFilename() const { return __FILE__; }
#endif

#if TEST_PARSE
  virtual ParseResultBase *Parse(const char *j, size_t length) const {
    JasonParseResult *pr = new JasonParseResult;
    try {
      parser p(std::string_view(j, length));
      pr->root = p.parse();
    } catch (...) {
      delete pr;
      return 0;
    }
    return pr;
  }
#endif

#if TEST_STATISTICS
  virtual bool Statistics(const ParseResultBase *parseResult,
                          Stat *stat) const {
    if (!parseResult)
      return false;
    const JasonParseResult *pr =
        static_cast<const JasonParseResult *>(parseResult);
    memset(stat, 0, sizeof(Stat));
    GenStat(*stat, pr->root);
    return true;
  }
#endif

#if TEST_CONFORMANCE
  virtual bool ParseDouble(const char *j, double *d) const {
    try {
      parser p(std::string_view(j, strlen(j)));
      Value root = p.parse();
      auto *arr = std::get_if<Value::Array>(&root.item);
      if (!arr || arr->size() != 1)
        return false;
      auto *num = std::get_if<double>(&(*arr)[0].item);
      if (!num)
        return false;
      *d = *num;
      return true;
    } catch (...) {
    }
    return false;
  }

  virtual bool ParseString(const char *j, std::string &s) const {
    try {
      parser p(std::string_view(j, strlen(j)));
      Value root = p.parse();
      auto *arr = std::get_if<Value::Array>(&root.item);
      if (!arr || arr->size() != 1)
        return false;
      auto *str = std::get_if<std::string>(&(*arr)[0].item);
      if (!str)
        return false;
      s = *str;
      return true;
    } catch (...) {
    }
    return false;
  }
#endif
};

REGISTER_TEST(JasonTest);
