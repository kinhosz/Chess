#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct TestCase {
  std::string name;
  std::function<void()> fn;
};

inline std::vector<TestCase>& allTests() {
  static std::vector<TestCase> tests;
  return tests;
}

struct TestRegistrar {
  TestRegistrar(const std::string &name, std::function<void()> fn) {
    allTests().push_back({name, fn});
  }
};

inline int g_failures = 0;
inline int g_assertions = 0;
inline std::vector<std::string>* g_currentTestFailures = nullptr;

inline void reportFailure(const std::string &file, int line, const std::string &msg) {
  g_failures++;
  std::ostringstream oss;
  oss << file << ":" << line << ": " << msg;
  if(g_currentTestFailures) g_currentTestFailures->push_back(oss.str());
}

// Registers a test function under `name`, run by runAllTests() in tests/main.cpp.
#define TEST(name) \
  void test_##name(); \
  static TestRegistrar registrar_##name(#name, test_##name); \
  void test_##name()

#define CHECK(cond) \
  do { \
    g_assertions++; \
    if(!(cond)) reportFailure(__FILE__, __LINE__, "CHECK failed: " #cond); \
  } while(0)

#define CHECK_EQ(a, b) \
  do { \
    g_assertions++; \
    auto _a = (a); auto _b = (b); \
    if(!(_a == _b)) { \
      std::ostringstream oss; \
      oss << #a " == " #b " (got " << _a << ", expected " << _b << ")"; \
      reportFailure(__FILE__, __LINE__, oss.str()); \
    } \
  } while(0)

#define CHECK_NEAR(a, b, eps) \
  do { \
    g_assertions++; \
    double _a = (a); double _b = (b); \
    if(std::abs(_a - _b) > (eps)) { \
      std::ostringstream oss; \
      oss << #a " ~= " #b " (got " << _a << ", expected " << _b << ", eps=" << (eps) << ")"; \
      reportFailure(__FILE__, __LINE__, oss.str()); \
    } \
  } while(0)

inline int runAllTests() {
  std::vector<std::pair<std::string, std::vector<std::string>>> failedTests;

  int dotsOnLine = 0;
  for(auto &t: allTests()) {
    std::vector<std::string> failures;
    g_currentTestFailures = &failures;
    t.fn();
    g_currentTestFailures = nullptr;

    std::cerr << (failures.empty() ? "." : "F");
    if(++dotsOnLine == 60) { std::cerr << "\n"; dotsOnLine = 0; }

    if(!failures.empty()) failedTests.push_back({t.name, failures});
  }
  if(dotsOnLine != 0) std::cerr << "\n";

  if(!failedTests.empty()) {
    std::cerr << "\nFailures:\n";
    for(auto &ft: failedTests) {
      std::cerr << "  " << ft.first << "\n";
      for(auto &msg: ft.second) std::cerr << "    " << msg << "\n";
    }
  }

  std::cerr << "\n" << allTests().size() << " tests, " << g_assertions
             << " assertions, " << g_failures << " failures\n";
  return g_failures == 0 ? 0 : 1;
}

#endif
