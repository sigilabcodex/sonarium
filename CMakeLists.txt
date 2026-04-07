cmake_minimum_required(VERSION 3.20)
project(sonarium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sonarium_core
    src/core/stft/stft_engine.cpp
    src/core/processors/spectral_gain_mask_processor.cpp
    src/engine/engine_facade.cpp
)

target_include_directories(sonarium_core PUBLIC src)

add_executable(sonarium_offline_harness src/app/offline_harness.cpp)
target_link_libraries(sonarium_offline_harness PRIVATE sonarium_core)

add_executable(sonarium_spectral_interaction_prototype src/app/spectral_interaction_prototype.cpp)
target_link_libraries(sonarium_spectral_interaction_prototype PRIVATE sonarium_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(sonarium_tests
        tests/test_main.cpp
        tests/test_stft.cpp
        tests/test_processor.cpp
    )
    target_link_libraries(sonarium_tests PRIVATE sonarium_core)
    add_test(NAME sonarium_tests COMMAND sonarium_tests)
endif()
