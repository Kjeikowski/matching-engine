add_library(trading_engine
    domain/Order.cpp
    domain/Trade.cpp
    domain/OrderBook.cpp
    domain/MatchingEngine.cpp
)

target_include_directories(trading_engine
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
