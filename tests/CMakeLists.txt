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

add_executable(TradeTest
    unit/TradeTest.cpp
)

target_link_libraries(TradeTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(TradeTest)

add_executable(OrderBookTest
    unit/OrderBookTest.cpp
)

target_link_libraries(OrderBookTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(OrderBookTest)
