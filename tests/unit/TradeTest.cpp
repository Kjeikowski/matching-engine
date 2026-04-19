#include <gtest/gtest.h>

#include <stdexcept>

#include "domain/Trade.hpp"

using namespace trading::domain;

namespace {

TEST(TradeTest, ConstructorPopulatesAllFieldsCorrectly) {
  const TradeId tradeId{1};
  const OrderId buyId{100};
  const OrderId sellId{200};
  const Price price{105};
  const Quantity qty{10};
  const Timestamp ts{123456789};

  const Trade trade{tradeId, buyId, sellId, price, qty, ts};

  EXPECT_EQ(trade.getTradeId().value, tradeId.value);
  EXPECT_EQ(trade.getBuyOrderId().value, buyId.value);
  EXPECT_EQ(trade.getSellOrderId().value, sellId.value);
  EXPECT_EQ(trade.getPrice().value, price.value);
  EXPECT_EQ(trade.getQuantity().value, qty.value);
  EXPECT_EQ(trade.getTimestamp().value, ts.value);
}

TEST(TradeTest, ConstructorThrowsWhenTradeIdIsZero) {
  EXPECT_THROW((Trade{TradeId{0}, OrderId{100}, OrderId{200}, Price{105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenBuyOrderIdIsZero) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{0}, OrderId{200}, Price{105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenSellOrderIdIsZero) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{0}, Price{105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenBuyAndSellOrderIdsMatch) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{100}, Price{105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenPriceIsZero) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{200}, Price{0},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenQuantityIsZero) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{200}, Price{105},
                      Quantity{0}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenTimestampIsZero) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{200}, Price{105},
                      Quantity{10}, Timestamp{0}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenTradeIdIsNegative) {
  EXPECT_THROW((Trade{TradeId{-1}, OrderId{100}, OrderId{200}, Price{105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenPriceIsNegative) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{200}, Price{-105},
                      Quantity{10}, Timestamp{123456789}}),
               std::invalid_argument);
}

TEST(TradeTest, ConstructorThrowsWhenQuantityIsNegative) {
  EXPECT_THROW((Trade{TradeId{1}, OrderId{100}, OrderId{200}, Price{105},
                      Quantity{-10}, Timestamp{123456789}}),
               std::invalid_argument);
}

}  // namespace
