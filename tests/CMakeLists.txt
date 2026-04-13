include(GoogleTest)

add_executable(OrderTest
    unit/OrderTest.cpp
)

target_link_libraries(OrderTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(OrderTest)

add_executable(TraderTest
    unit/TraderTest.cpp
)

target_link_libraries(TraderTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(TraderTest)
