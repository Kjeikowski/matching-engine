#include <gtest/gtest.h>

#include "domain/MatchingEngine.hpp"
#include "domain/Order.hpp"
#include "domain/OrderBook.hpp"

using namespace trading::domain;

namespace {

// Helper functions
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

// ============================================================================
// CONSTRUCTION AND INITIAL STATE
// ============================================================================

TEST_F(MatchingEngineTest, ConstructsWithEmptyState) {
  EXPECT_EQ(engine.getTotalOrderBookCount(), 0U);
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 0);
}

// ============================================================================
// SINGLE SYMBOL TESTS
// ============================================================================

TEST_F(MatchingEngineTest, SubmitSingleBuyOrder) {
  const auto trades =
      engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(engine.getTotalOrderBookCount(), 1U);
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_TRUE(engine.hasOrderBook(aapl));
}

TEST_F(MatchingEngineTest, SubmitSingleSellOrder) {
  const auto trades =
      engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::SELL, 100, 150));

  EXPECT_TRUE(trades.empty());
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

// ============================================================================
// MATCHING AND TRADES
// ============================================================================

TEST_F(MatchingEngineTest, SimpleBuyAndSellMatch) {
  // Submit a sell order first
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  // Submit matching buy order
  const auto trades =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getQuantity().value, 100);
  EXPECT_EQ(trades[0].getPrice().value, 150);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, PartialFillWithMultipleOrders) {
  // Submit two sell orders
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 60, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 40, 150));

  // Submit larger buy order
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].getQuantity().value, 60);
  EXPECT_EQ(trades[1].getQuantity().value, 40);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, MultiLevelMatching) {
  // Build a sell side at multiple price levels
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 151));

  // Submit large buy order that crosses multiple levels
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 151));

  EXPECT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].getPrice().value, 150);
  EXPECT_EQ(trades[1].getPrice().value, 151);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, NoMatchWhenBidBelowAsk) {
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  const auto trades =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 149));

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

TEST_F(MatchingEngineTest, SellSideMakesMultipleMatches) {
  // Build a buy side
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 60, 150));
  engine.submitOrder(makeLimitOrder(2, 1002, aapl, Side::BUY, 40, 150));

  // Submit large sell order
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 2001, aapl, Side::SELL, 100, 150));

  EXPECT_EQ(trades.size(), 2U);
  EXPECT_EQ(engine.getTotalTradeCount(), 2U);
  EXPECT_EQ(engine.getTotalTradeVolume(), 100);
}

TEST_F(MatchingEngineTest, MarketOrderExecutesImmediately) {
  // Create liquidity
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));

  // Market buy order
  const auto trades =
      engine.submitOrder(makeMarketOrder(2, 1001, aapl, Side::BUY, 100));

  EXPECT_EQ(trades.size(), 1U);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, MarketOrderPartiallyFilled) {
  // Create insufficient liquidity
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));

  // Market buy order for more
  const auto trades =
      engine.submitOrder(makeMarketOrder(2, 1001, aapl, Side::BUY, 100));

  EXPECT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getQuantity().value, 50);
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

// ============================================================================
// CANCELLATION TESTS
// ============================================================================

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

  // The sell order (2001) is now partially filled (50 out of 100) and resting
  EXPECT_TRUE(engine.cancelOrder(OrderId{2001}));
}

TEST_F(MatchingEngineTest, CancelPreventsFutureMatches) {
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));
  EXPECT_TRUE(engine.cancelOrder(OrderId{1001}));

  // Now submit a sell order - it shouldn't match with the canceled buy
  const auto trades =
      engine.submitOrder(makeLimitOrder(2, 2001, aapl, Side::SELL, 100, 150));

  EXPECT_TRUE(trades.empty());
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
}

// ============================================================================
// MULTIPLE SYMBOL TESTS
// ============================================================================

TEST_F(MatchingEngineTest, MultipleSymbolsAreIndependent) {
  // AAPL orders
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 100, 150));

  // MSFT orders
  engine.submitOrder(makeLimitOrder(2, 2001, msft, Side::SELL, 100, 200));

  // They shouldn't match
  EXPECT_EQ(engine.getTotalTradeCount(), 0U);
  EXPECT_EQ(engine.getTotalOrderBookCount(), 2U);
}

TEST_F(MatchingEngineTest, MultiSymbolMatching) {
  // Create matching pairs on different symbols
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  const auto aapl_trades =
      engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 100, 150));

  engine.submitOrder(makeLimitOrder(3, 2002, msft, Side::SELL, 50, 200));
  const auto msft_trades =
      engine.submitOrder(makeLimitOrder(4, 1002, msft, Side::BUY, 50, 200));

  EXPECT_EQ(aapl_trades.size(), 1U);
  EXPECT_EQ(msft_trades.size(), 1U);
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

// ============================================================================
// METRICS AND STATE TESTS
// ============================================================================

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

  // Cancellation doesn't remove completed trades from metrics
  engine.cancelOrder(OrderId{1001});  // Already matched
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

// ============================================================================
// ERROR CASES AND EDGE CASES
// ============================================================================

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

// ============================================================================
// PRICE-TIME PRIORITY TESTS
// ============================================================================

TEST_F(MatchingEngineTest, BestBidWinsOnBuySide) {
  // Add two sell orders at different prices
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 149));

  // Submit matching buy - should match with 149 (best ask)
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  EXPECT_EQ(trades[0].getPrice().value, 149);  // Best ask price
}

TEST_F(MatchingEngineTest, BestAskWinsOnSellSide) {
  // Add two buy orders at different prices
  engine.submitOrder(makeLimitOrder(1, 1001, aapl, Side::BUY, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 1002, aapl, Side::BUY, 50, 151));

  // Submit matching sell - should match with 151 (best bid)
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 2001, aapl, Side::SELL, 100, 149));

  EXPECT_EQ(trades[0].getPrice().value, 151);  // Best bid price
}

TEST_F(MatchingEngineTest, TimeOrderPriority) {
  // Add two sell orders at same price
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 50, 150));
  engine.submitOrder(makeLimitOrder(2, 2002, aapl, Side::SELL, 50, 150));

  // Submit matching buy
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 1001, aapl, Side::BUY, 100, 150));

  // First submitted should match first (order 2001)
  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].getSellOrderId().value, 2001);
  EXPECT_EQ(trades[1].getSellOrderId().value, 2002);
}

// ============================================================================
// COMPLEX SCENARIOS
// ============================================================================

TEST_F(MatchingEngineTest, ComplexScenarioWithCancelsAndMatches) {
  // Add liquidity
  engine.submitOrder(makeLimitOrder(1, 2001, aapl, Side::SELL, 100, 150));
  engine.submitOrder(makeLimitOrder(2, 1001, aapl, Side::BUY, 50, 150));

  // Should have one match
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);

  // Cancel the partially filled sell order
  EXPECT_TRUE(engine.cancelOrder(OrderId{2001}));

  // Add a new buyer - no match now
  const auto trades =
      engine.submitOrder(makeLimitOrder(3, 1002, aapl, Side::BUY, 50, 150));
  EXPECT_TRUE(trades.empty());

  // Overall should still have only 1 trade
  EXPECT_EQ(engine.getTotalTradeCount(), 1U);
}

TEST_F(MatchingEngineTest, LargeVolumeScenario) {
  // Create many orders
  const std::int64_t numOrders = 100;
  const std::int64_t qty = 10;

  for (std::int64_t i = 0; i < numOrders; ++i) {
    engine.submitOrder(makeLimitOrder(i, 2000 + i, aapl, Side::SELL, qty, 150));
  }

  // Submit large buy order
  const auto trades = engine.submitOrder(
      makeLimitOrder(1000, 1000, aapl, Side::BUY, numOrders * qty, 150));

  EXPECT_EQ(trades.size(), static_cast<std::size_t>(numOrders));
  EXPECT_EQ(engine.getTotalTradeCount(), static_cast<std::size_t>(numOrders));
  EXPECT_EQ(engine.getTotalTradeVolume(), numOrders * qty);
}
