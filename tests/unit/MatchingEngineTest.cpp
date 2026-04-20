#include <gtest/gtest.h>

#include "domain/MatchingEngine.hpp"
#include "domain/Order.hpp"
#include "domain/OrderBook.hpp"

using namespace trading::domain;

namespace {

Order makeLimitOrder(std::int64_t seq, std::int64_t id, const Symbol& symbol,
                     Side side, std::int64_t qty, std::int64_t price) {
  return Order{SequenceNumber{seq}, OrderId{id},   symbol, side,
               Type::LIMIT,         Quantity{qty}, Price{price}};
}

Order makeMarketOrder(std::int64_t seq, std::int64_t id, const Symbol& symbol,
                      Side side, std::int64_t qty) {
  return Order{SequenceNumber{seq}, OrderId{id}, symbol, side, Type::MARKET,
               Quantity{qty}};
}

}  // namespace

class MatchingEngineTest : public ::testing::Test {
 protected:
  MatchingEngine engine;
  Symbol aapl{"AAPL"};
  Symbol msft{"MSFT"};
};

TEST_F(MatchingEngineTest, ConstructsWithEmptyState) {
  EXPECT_EQ(engine.getTotalOrderBookCount(), 0U);
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 0);
}

TEST_F(MatchingEngineTest, SubmitSingleBuyOrder) {
  const auto report =
      engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getSymbol().value, "AAPL");
  EXPECT_EQ(report.getSide(), Side::BUY);
  EXPECT_EQ(report.getOrderType(), Type::LIMIT);
  EXPECT_EQ(report.getOriginalQuantity().value, 100);
  EXPECT_EQ(report.getRemainingQuantity().value, 100);
  EXPECT_TRUE(report.getTrades().empty());
  EXPECT_EQ(engine.getTotalOrderBookCount(), 1U);
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_TRUE(engine.hasOrderBook(aapl));
}

TEST_F(MatchingEngineTest, SubmitSingleSellOrder) {
  const auto report =
      engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::SELL, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getSymbol().value, "AAPL");
  EXPECT_EQ(report.getSide(), Side::SELL);
  EXPECT_EQ(report.getOrderType(), Type::LIMIT);
  EXPECT_EQ(report.getOriginalQuantity().value, 100);
  EXPECT_EQ(report.getRemainingQuantity().value, 100);
  EXPECT_TRUE(report.getTrades().empty());
  EXPECT_EQ(engine.getTotalOrderBookCount(), 1U);
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

TEST_F(MatchingEngineTest, GetOrderBookAfterSubmit) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  auto book = engine.getOrderBook(aapl);
  ASSERT_TRUE(book.has_value());
  ASSERT_TRUE(book.value());
  EXPECT_EQ(book.value()->getSymbol().value, "AAPL");
}

TEST_F(MatchingEngineTest, GetOrderBookReturnsNothingForNonExistent) {
  auto book = engine.getOrderBook(aapl);
  EXPECT_FALSE(book.has_value());
}

TEST_F(MatchingEngineTest, HasOrderBookReturnsFalseForNonExistent) {
  EXPECT_FALSE(engine.hasOrderBook(aapl));
}

TEST_F(MatchingEngineTest, SimpleBuyAndSellMatch) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  const auto report =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 1U);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 100);
  EXPECT_EQ(report.getTrades()[0].getPrice().value, 150);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, PartialFillWithMultipleOrders) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 60, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 40, 150));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 2U);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 60);
  EXPECT_EQ(report.getTrades()[1].getQuantity().value, 40);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, MultiLevelMatching) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 151));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 151));

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 2U);
  EXPECT_EQ(report.getTrades()[0].getPrice().value, 150);
  EXPECT_EQ(report.getTrades()[1].getPrice().value, 151);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, NoMatchWhenBidBelowAsk) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  const auto report =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 149));

  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 100);
  EXPECT_TRUE(report.getTrades().empty());
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

TEST_F(MatchingEngineTest, SellSideMakesMultipleMatches) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 60, 150));
  engine.submitOrder(makeLimitOrder(2, 1002, aapl, Side::BUY, 40, 150));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 2001, aapl, Side::SELL, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getOrderId().value, 2001);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 2U);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, MarketOrderExecutesImmediately) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  const auto report =
      engine.submitOrder(makeMarketOrder(2, 1001, aapl, Side::BUY, 100));

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 1U);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, MarketOrderPartiallyFilled) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));

  const auto report =
      engine.submitOrder(makeMarketOrder(2, 1001, aapl, Side::BUY, 100));

  EXPECT_EQ(report.getType(), ExecutionReportType::PARTIAL_FILL);
  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getRemainingQuantity().value, 50);
  ASSERT_EQ(report.getTrades().size(), 1U);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 50);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, CancelActiveOrder) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_TRUE(engine.cancelOrder(OrderId{1001}));
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

TEST_F(MatchingEngineTest, CancelNonExistentOrder) {
  EXPECT_FALSE(engine.cancelOrder(OrderId{9999}));
}

TEST_F(MatchingEngineTest, CancelAfterPartialFill) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 50, 150));

  EXPECT_TRUE(engine.cancelOrder(OrderId{2001}));
}

TEST_F(MatchingEngineTest, CancelPreventsFutureMatches) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));
  EXPECT_TRUE(engine.cancelOrder(OrderId{1001}));

  const auto report =
      engine.submitOrder(makeLimitOrder(2, 2001, aapl, Side::SELL, 100, 150));

  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_TRUE(report.getTrades().empty());
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

TEST_F(MatchingEngineTest, MultipleSymbolsAreIndependent) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  engine.submitOrder(makeLimitOrder(2, 2001, msft, Side::SELL, 100, 200));

  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_EQ(engine.getTotalOrderBookCount(), 2U);
}

TEST_F(MatchingEngineTest, MultiSymbolMatching) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  const auto aapl_report =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  engine.submitOrder(makeLimitOrder(3, 2002, msft, Side::SELL, 50, 200));
  const auto msft_report =
      engine.submitOrder(makeLimitOrder(4, 1002, msft, Side::BUY, 50, 200));

  EXPECT_EQ(aapl_report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(msft_report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 150);
}

TEST_F(MatchingEngineTest, EachSymbolHasOwnOrderBook) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1002, msft, Side::BUY, 100, 200));

  auto aapl_book = engine.getOrderBook(aapl);
  auto msft_book = engine.getOrderBook(msft);

  ASSERT_TRUE(aapl_book.has_value());
  ASSERT_TRUE(msft_book.has_value());
  EXPECT_NE(&*aapl_book, &*msft_book);
}

TEST_F(MatchingEngineTest, TotalTradeCountAcrossSymbols) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  engine.submitOrder(makeLimitOrder(3, 2002, msft, Side::SELL, 50, 200));
  engine.submitOrder(makeLimitOrder(4, 1002, msft, Side::BUY, 50, 200));

  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
}

TEST_F(MatchingEngineTest, TotalTradeVolumeAcrossSymbols) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  engine.submitOrder(makeLimitOrder(3, 2002, msft, Side::SELL, 75, 200));
  engine.submitOrder(makeLimitOrder(4, 1002, msft, Side::BUY, 75, 200));

  EXPECT_EQ(engine.getTotalTradeVolume(), 175);
}

TEST_F(MatchingEngineTest, MetricsAfterCancellation) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(engine.getTotalTradeCount(), 1U);

  engine.cancelOrder(OrderId{1001});
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, DuplicateOrderIdThrows) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_THROW(engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 50, 150)),
               std::invalid_argument);
}

TEST_F(MatchingEngineTest, ZeroQuantityOrderThrows) {
  EXPECT_THROW(
      engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 0, 150)),
      std::invalid_argument);
}

TEST_F(MatchingEngineTest, LimitOrderWithoutPriceThrows) {
  EXPECT_THROW(
      Order(SequenceNumber{1}, OrderId{1001}, aapl, Side::BUY, Type::LIMIT,
            Quantity{100}, std::nullopt),
      std::invalid_argument);
}

TEST_F(MatchingEngineTest, BestBidWinsOnBuySide) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 149));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(report.getTrades()[0].getPrice().value, 149);
}

TEST_F(MatchingEngineTest, BestAskWinsOnSellSide) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 1002, aapl, Side::BUY, 50, 151));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 2001, aapl, Side::SELL, 100, 149));

  EXPECT_EQ(report.getTrades()[0].getPrice().value, 151);
}

TEST_F(MatchingEngineTest, TimeOrderPriority) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 150));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  ASSERT_EQ(report.getTrades().size(), 2U);
  EXPECT_EQ(report.getTrades()[0].getSellOrderId().value, 2001);
  EXPECT_EQ(report.getTrades()[1].getSellOrderId().value, 2002);
}

TEST_F(MatchingEngineTest, ComplexScenarioWithCancelsAndMatches) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 50, 150));

  EXPECT_EQ(engine.getTotalTradeCount(), 1U);

  EXPECT_TRUE(engine.cancelOrder(OrderId{2001}));

  const auto report =
      engine.submitOrder(makeLimitOrder(3, 1002, aapl, Side::BUY, 50, 150));
  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_TRUE(report.getTrades().empty());

  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, LargeVolumeScenario) {
  const std::int64_t numOrders = 100;
  const std::int64_t qty = 10;

  for (std::int64_t i = 0; i < numOrders; ++i) {
    engine.submitOrder(makeLimitOrder(i, 2000 + i, aapl, Side::SELL, qty, 150));
  }

  const auto report = engine.submitOrder(
      makeLimitOrder(1000, 1000, aapl, Side::BUY, numOrders * qty, 150));

  EXPECT_EQ(report.getTrades().size(), static_cast<std::size_t>(numOrders));
  EXPECT_EQ(engine.getTotalTradeCount(), static_cast<std::size_t>(numOrders));
  EXPECT_EQ(engine.getTotalTradeVolume(), numOrders * qty);
}
