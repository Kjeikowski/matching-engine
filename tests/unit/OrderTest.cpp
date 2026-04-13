#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>

#include "domain/Order.hpp"

using namespace trading::domain;

namespace {

Order makeLimitOrder() {
  return Order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
               Type::LIMIT, Quantity{50}, Price{3456});
}

Order makeMarketOrder() {
  return Order(SequenceNumber{1}, OrderId{101}, Symbol{"AAPL"}, Side::SELL,
               Type::MARKET, Quantity{25}, std::nullopt);
}

}  // namespace

TEST(OrderTest, ValidLimitOrderInitializesAllFieldsCorrectly) {
  Order order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
              Type::LIMIT, Quantity{50}, Price{3456});

  EXPECT_EQ(order.getSequenceNumber().value, 1);
  EXPECT_EQ(order.getOrderId().value, 100);
  EXPECT_EQ(order.getSymbol().value, "AAPL");
  EXPECT_EQ(order.getSide(), Side::BUY);
  EXPECT_EQ(order.getType(), Type::LIMIT);
  EXPECT_EQ(order.getStatus(), Status::NEW);
  EXPECT_EQ(order.getOriginalQuantity().value, 50);
  EXPECT_EQ(order.getRemainingQuantity().value, 50);
  EXPECT_EQ(order.getFilledQuantity().value, 0);

  ASSERT_TRUE(order.getPrice().has_value());
  EXPECT_EQ(order.getPrice()->value, 3456);
  EXPECT_TRUE(order.hasPrice());
  EXPECT_TRUE(order.isActive());
  EXPECT_FALSE(order.isFilled());
  EXPECT_FALSE(order.isCanceled());
}

TEST(OrderTest, ValidMarketOrderInitializesWithoutPrice) {
  Order order = makeMarketOrder();

  EXPECT_EQ(order.getType(), Type::MARKET);
  EXPECT_FALSE(order.getPrice().has_value());
  EXPECT_FALSE(order.hasPrice());
  EXPECT_EQ(order.getStatus(), Status::NEW);
  EXPECT_TRUE(order.isActive());
}

TEST(OrderTest, ConstructorRejectsNegativeSequenceNumber) {
  EXPECT_THROW(Order(SequenceNumber{-1}, OrderId{100}, Symbol{"AAPL"},
                     Side::BUY, Type::LIMIT, Quantity{50}, Price{3456}),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsNonPositiveOrderId) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{0}, Symbol{"AAPL"}, Side::BUY,
                     Type::LIMIT, Quantity{50}, Price{3456}),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsEmptySymbol) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{100}, Symbol{""}, Side::BUY,
                     Type::LIMIT, Quantity{50}, Price{3456}),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsNonPositiveQuantity) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
                     Type::LIMIT, Quantity{0}, Price{3456}),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsLimitOrderWithoutPrice) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
                     Type::LIMIT, Quantity{50}, std::nullopt),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsLimitOrderWithNonPositivePrice) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
                     Type::LIMIT, Quantity{50}, Price{0}),
               std::invalid_argument);
}

TEST(OrderTest, ConstructorRejectsMarketOrderWithPrice) {
  EXPECT_THROW(Order(SequenceNumber{1}, OrderId{100}, Symbol{"AAPL"}, Side::BUY,
                     Type::MARKET, Quantity{50}, Price{3456}),
               std::invalid_argument);
}

TEST(OrderTest, FillPartiallyReducesRemainingQuantityAndUpdatesStatus) {
  Order order = makeLimitOrder();

  order.fill(Quantity{20});

  EXPECT_EQ(order.getRemainingQuantity().value, 30);
  EXPECT_EQ(order.getFilledQuantity().value, 20);
  EXPECT_EQ(order.getStatus(), Status::PARTIALLY_FILLED);
  EXPECT_TRUE(order.isActive());
  EXPECT_FALSE(order.isFilled());
  EXPECT_FALSE(order.isCanceled());
}

TEST(OrderTest, FillCompletelySetsFilledStatus) {
  Order order = makeLimitOrder();

  order.fill(Quantity{50});

  EXPECT_EQ(order.getRemainingQuantity().value, 0);
  EXPECT_EQ(order.getFilledQuantity().value, 50);
  EXPECT_EQ(order.getStatus(), Status::FILLED);
  EXPECT_FALSE(order.isActive());
  EXPECT_TRUE(order.isFilled());
  EXPECT_FALSE(order.isCanceled());
}

TEST(OrderTest, MultipleFillsAccumulateCorrectly) {
  Order order = makeLimitOrder();

  order.fill(Quantity{10});
  order.fill(Quantity{15});

  EXPECT_EQ(order.getRemainingQuantity().value, 25);
  EXPECT_EQ(order.getFilledQuantity().value, 25);
  EXPECT_EQ(order.getStatus(), Status::PARTIALLY_FILLED);
}

TEST(OrderTest, FillRejectsZeroExecutedQuantity) {
  Order order = makeLimitOrder();

  EXPECT_THROW(order.fill(Quantity{0}), std::invalid_argument);
}

TEST(OrderTest, FillRejectsNegativeExecutedQuantity) {
  Order order = makeLimitOrder();

  EXPECT_THROW(order.fill(Quantity{-1}), std::invalid_argument);
}

TEST(OrderTest, FillRejectsQuantityGreaterThanRemaining) {
  Order order = makeLimitOrder();

  EXPECT_THROW(order.fill(Quantity{51}), std::invalid_argument);
}

TEST(OrderTest, FillRejectsWhenOrderAlreadyFilled) {
  Order order = makeLimitOrder();
  order.fill(Quantity{50});

  EXPECT_THROW(order.fill(Quantity{1}), std::logic_error);
}

TEST(OrderTest, FillRejectsWhenOrderCanceled) {
  Order order = makeLimitOrder();
  order.cancel();

  EXPECT_THROW(order.fill(Quantity{1}), std::logic_error);
}

TEST(OrderTest, CancelChangesStatusToCanceled) {
  Order order = makeLimitOrder();

  order.cancel();

  EXPECT_EQ(order.getStatus(), Status::CANCELED);
  EXPECT_FALSE(order.isActive());
  EXPECT_FALSE(order.isFilled());
  EXPECT_TRUE(order.isCanceled());
  EXPECT_EQ(order.getRemainingQuantity().value, 50);
  EXPECT_EQ(order.getFilledQuantity().value, 0);
}

TEST(OrderTest, CancelAfterPartialFillIsAllowed) {
  Order order = makeLimitOrder();

  order.fill(Quantity{10});
  order.cancel();

  EXPECT_EQ(order.getStatus(), Status::CANCELED);
  EXPECT_EQ(order.getRemainingQuantity().value, 40);
  EXPECT_EQ(order.getFilledQuantity().value, 10);
  EXPECT_TRUE(order.isCanceled());
  EXPECT_FALSE(order.isActive());
}

TEST(OrderTest, CancelRejectsAlreadyCanceledOrder) {
  Order order = makeLimitOrder();
  order.cancel();

  EXPECT_THROW(order.cancel(), std::logic_error);
}

TEST(OrderTest, CancelRejectsFilledOrder) {
  Order order = makeLimitOrder();
  order.fill(Quantity{50});

  EXPECT_THROW(order.cancel(), std::logic_error);
}
