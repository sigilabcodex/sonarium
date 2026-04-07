#pragma once

#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> run;
};

std::vector<TestCase>& registry();
void register_test(std::string name, std::function<void()> fn);

#define SONARIUM_TEST(name)                                             \
    void name();                                                         \
    namespace {                                                          \
    const bool name##_registered = [] {                                 \
        register_test(#name, name);                                     \
        return true;                                                     \
    }();                                                                 \
    }                                                                    \
    void name()

#define REQUIRE_TRUE(condition, message)                   \
    do {                                                    \
        if (!(condition)) {                                 \
            throw std::runtime_error((message));            \
        }                                                   \
    } while (false)

#define REQUIRE_NEAR(actual, expected, tolerance, message)                               \
    do {                                                                                  \
        if (std::fabs((actual) - (expected)) > (tolerance)) {                            \
            throw std::runtime_error(std::string(message) + " actual=" +                \
                                     std::to_string(actual) + " expected=" +            \
                                     std::to_string(expected));                            \
        }                                                                                 \
    } while (false)
