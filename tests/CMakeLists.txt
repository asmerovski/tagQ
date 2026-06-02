enable_testing()

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(TEST_SOURCES
    test_audiofile.cpp
    test_tagsmanager.cpp
    test_integration.cpp
)

add_executable(tagQ_tests ${TEST_SOURCES})

target_link_libraries(tagQ_tests
    ${GTEST_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    tag
)

gtest_discover_tests(tagQ_tests)
