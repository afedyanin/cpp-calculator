#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Минимальный тестовый фреймворк без внешних зависимостей.
namespace test_framework {

struct TestCase {
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& Registry()
{
    static std::vector<TestCase> tests;
    return tests;
}

class Registration {
  public:
    Registration(std::string name, std::function<void()> body)
    {
        Registry().push_back(TestCase{std::move(name), std::move(body)});
    }
};

[[noreturn]] inline void Fail(std::string_view file, int line, const std::string& message)
{
    std::ostringstream stream;
    stream << file << ':' << line << ": " << message;
    throw std::runtime_error(stream.str());
}

// Прогоняет все зарегистрированные тесты.
// Возвращает 0, если все тесты прошли, и 1 в противном случае.
inline int RunAllTests()
{
    int failedCount = 0;

    for (const TestCase& test : Registry()) {
        try {
            test.body();
            std::cout << "[  PASSED  ] " << test.name << '\n';
        }
        catch (const std::exception& error) {
            ++failedCount;
            std::cout << "[  FAILED  ] " << test.name << '\n';
            std::cout << "    " << error.what() << '\n';
        }
    }

    const int totalCount = static_cast<int>(Registry().size());
    std::cout << '\n' << totalCount - failedCount << " passed, " << failedCount << " failed\n";

    return failedCount == 0 ? 0 : 1;
}

} // namespace test_framework

// Объявляет тест и регистрирует его для запуска в RunAllTests().
#define TEST(testName)                                                                             \
    void testName##Body();                                                                         \
    [[maybe_unused]] const ::test_framework::Registration testName##Registrar(#testName,           \
                                                                              testName##Body);     \
    void testName##Body()

#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            ::test_framework::Fail(__FILE__, __LINE__, "CHECK failed: " #condition);               \
        }                                                                                          \
    } while (false)

#define CHECK_EQ(actual, expected)                                                                 \
    do {                                                                                           \
        const auto& checkEqActual = (actual);                                                      \
        const auto& checkEqExpected = (expected);                                                  \
        if (!(checkEqActual == checkEqExpected)) {                                                 \
            std::ostringstream checkEqMessage;                                                     \
            checkEqMessage << "CHECK_EQ failed: " #actual " == " #expected                         \
                           << "\n      actual:   " << checkEqActual                                \
                           << "\n      expected: " << checkEqExpected;                             \
            ::test_framework::Fail(__FILE__, __LINE__, checkEqMessage.str());                      \
        }                                                                                          \
    } while (false)

#define CHECK_NEAR(actual, expected, tolerance)                                                    \
    do {                                                                                           \
        const double checkNearActual = (actual);                                                   \
        const double checkNearExpected = (expected);                                               \
        const double checkNearTolerance = (tolerance);                                             \
        if (std::fabs(checkNearActual - checkNearExpected) > checkNearTolerance) {                 \
            std::ostringstream checkNearMessage;                                                   \
            checkNearMessage << "CHECK_NEAR failed: " #actual " ~= " #expected                     \
                             << "\n      actual:    " << checkNearActual                           \
                             << "\n      expected:  " << checkNearExpected                         \
                             << "\n      tolerance: " << checkNearTolerance;                       \
            ::test_framework::Fail(__FILE__, __LINE__, checkNearMessage.str());                    \
        }                                                                                          \
    } while (false)
