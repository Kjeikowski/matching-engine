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

add_executable(MatchingEngineTest
    unit/MatchingEngineTest.cpp
)

target_link_libraries(MatchingEngineTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(MatchingEngineTest)

add_executable(EngineConfigTest
    unit/EngineConfigTest.cpp
)

target_link_libraries(EngineConfigTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(EngineConfigTest)

add_executable(ExecutionReportTest
    unit/ExecutionReportTest.cpp
)

target_link_libraries(ExecutionReportTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(ExecutionReportTest)

add_executable(ServiceConfigTest
    unit/ServiceConfigTest.cpp
)

target_link_libraries(ServiceConfigTest
    PRIVATE
    trading_engine
    gtest_main
)

gtest_discover_tests(ServiceConfigTest)
