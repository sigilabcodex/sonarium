#include "test_framework.h"

#include <exception>
#include <iostream>

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

void register_test(std::string name, std::function<void()> fn) {
    registry().push_back({std::move(name), std::move(fn)});
}

extern void register_stft_tests();
extern void register_processor_tests();

int main() {
    register_stft_tests();
    register_processor_tests();

    std::size_t failures = 0;
    for (const auto& test : registry()) {
        try {
            test.run();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& ex) {
            ++failures;
            std::cout << "[FAIL] " << test.name << " :: " << ex.what() << '\n';
        }
    }

    if (failures > 0) {
        std::cout << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All tests passed (" << registry().size() << ").\n";
    return 0;
}
