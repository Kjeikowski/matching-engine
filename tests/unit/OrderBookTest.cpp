#include <gtest/gtest.h>

#include <optional>

#include "domain/Order.hpp"
#include "domain/OrderBook.hpp"

using namespace trading::domain;

namespace {

Order makeLimitOrder(std::int64_t seq, std::int64_t id, Side side,
                     std::int64_t qty, std::int64_t price,
                     const std::string& symbol = "AAPL") {
  return Order{SequenceNumber{seq}, OrderId{id},   Symbol{symbol}, side,
               Type::LIMIT,         Quantity{qty}, Price{price}};
}

Order makeMarketOrder(std::int64_t seq, std::int64_t id, Side side,
                      std::int64_t qty, const std::string& symbol = "AAPL") {
  return Order{SequenceNumber{seq}, OrderId{id},  Symbol{symbol}, side,
               Type::MARKET,        Quantity{qty}};
}

}  // namespace

TEST(OrderBookTest, ConstructsForSingleSymbol) {
  OrderBook book{Symbol{"AAPL"}};

  EXPECT_EQ(book.getSymbol().value, "AAPL");
  EXPECT_FALSE(book.getBestBid().has_value());
  EXPECT_FALSE(book.getBestAsk().has_value());
}

TEST(OrderBookTest, AddBuyLimitOrderToEmptyBook) {
  OrderBook book{Symbol{"AAPL"}};

  const auto trades =
      book.addOrder(makeLimitOrder(1, 1001, Side::BUY, 10, 101));

  EXPECT_TRUE(trades.empty());
  ASSERT_TRUE(book.getBestBid().has_value());
  EXPECT_EQ(book.getBestBid()->value, 101);
  EXPECT_EQ(book.getBidLevelCount(), 1U);
  EXPECT_EQ(book.getBidOrderCount(), 1U);
  EXPECT_FALSE(book.getBestAsk().has_value());
}

TEST(OrderBookTest, AddSellLimitOrderToEmptyBook) {
  OrderBook book{Symbol{"AAPL"}};

  const auto trades =
      book.addOrder(makeLimitOrder(1, 1002, Side::SELL, 12, 105));

  EXPECT_TRUE(trades.empty());
  ASSERT_TRUE(book.getBestAsk().has_value());
  EXPECT_EQ(book.getBestAsk()->value, 105);
  EXPECT_EQ(book.getAskLevelCount(), 1U);
  EXPECT_EQ(book.getAskOrderCount(), 1U);
  EXPECT_FALSE(book.getBestBid().has_value());
}

TEST(OrderBookTest, BuyOrderMatchesRestingAsk) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 2001, Side::SELL, 10, 100));

  const auto trades =
      book.addOrder(makeLimitOrder(2, 2002, Side::BUY, 10, 101));

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getBuyOrderId().value, 2002);
  EXPECT_EQ(trades[0].getSellOrderId().value, 2001);
  EXPECT_EQ(trades[0].getPrice().value, 100);
  EXPECT_EQ(trades[0].getQuantity().value, 10);
  EXPECT_FALSE(book.getBestAsk().has_value());
  EXPECT_FALSE(book.getBestBid().has_value());
}

TEST(OrderBookTest, SellOrderMatchesRestingBid) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 3001, Side::BUY, 8, 101));

  const auto trades =
      book.addOrder(makeLimitOrder(2, 3002, Side::SELL, 8, 100));

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getBuyOrderId().value, 3001);
  EXPECT_EQ(trades[0].getSellOrderId().value, 3002);
  EXPECT_EQ(trades[0].getPrice().value, 101);
  EXPECT_EQ(trades[0].getQuantity().value, 8);
  EXPECT_FALSE(book.getBestBid().has_value());
  EXPECT_FALSE(book.getBestAsk().has_value());
}

TEST(OrderBookTest, PartialFillLeavesRemainderOnRestingBookSide) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 4001, Side::SELL, 10, 100));

  const auto trades = book.addOrder(makeLimitOrder(2, 4002, Side::BUY, 4, 105));

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getQuantity().value, 4);
  ASSERT_TRUE(book.getBestAsk().has_value());
  EXPECT_EQ(book.getBestAsk()->value, 100);
  EXPECT_EQ(book.getAskOrderCount(), 1U);
  EXPECT_FALSE(book.getBestBid().has_value());
}

TEST(OrderBookTest, UnfilledLimitRemainderRestsOnBook) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 5001, Side::SELL, 5, 100));

  const auto trades = book.addOrder(makeLimitOrder(2, 5002, Side::BUY, 9, 101));

  ASSERT_EQ(trades.size(), 1U);
  EXPECT_EQ(trades[0].getQuantity().value, 5);
  ASSERT_TRUE(book.getBestBid().has_value());
  EXPECT_EQ(book.getBestBid()->value, 101);
  EXPECT_EQ(book.getBidOrderCount(), 1U);
  EXPECT_FALSE(book.getBestAsk().has_value());
}

TEST(OrderBookTest, MarketOrderConsumesLiquidityAndDoesNotRest) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 6001, Side::SELL, 5, 100));
  book.addOrder(makeLimitOrder(2, 6002, Side::SELL, 4, 101));

  const auto trades = book.addOrder(makeMarketOrder(3, 6003, Side::BUY, 7));

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].getPrice().value, 100);
  EXPECT_EQ(trades[0].getQuantity().value, 5);
  EXPECT_EQ(trades[1].getPrice().value, 101);
  EXPECT_EQ(trades[1].getQuantity().value, 2);
  ASSERT_TRUE(book.getBestAsk().has_value());
  EXPECT_EQ(book.getBestAsk()->value, 101);
  EXPECT_EQ(book.getAskOrderCount(), 1U);
  EXPECT_FALSE(book.getBestBid().has_value());
}

TEST(OrderBookTest, CancelExistingOrderRemovesItFromBook) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 7001, Side::BUY, 10, 99));

  EXPECT_TRUE(book.cancelOrder(OrderId{7001}));
  EXPECT_FALSE(book.getBestBid().has_value());
  EXPECT_EQ(book.getBidOrderCount(), 0U);
}

TEST(OrderBookTest, CancelMissingOrderReturnsFalse) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 8001, Side::BUY, 10, 99));

  EXPECT_FALSE(book.cancelOrder(OrderId{9999}));
  ASSERT_TRUE(book.getBestBid().has_value());
  EXPECT_EQ(book.getBestBid()->value, 99);
}

TEST(OrderBookTest, EnforcesPriceTimePriorityWithinSamePriceLevel) {
  OrderBook book{Symbol{"AAPL"}};
  book.addOrder(makeLimitOrder(1, 9001, Side::SELL, 5, 100));
  book.addOrder(makeLimitOrder(2, 9002, Side::SELL, 5, 100));

  const auto trades = book.addOrder(makeLimitOrder(3, 9003, Side::BUY, 6, 100));

  ASSERT_EQ(trades.size(), 2U);
  EXPECT_EQ(trades[0].getSellOrderId().value, 9001);
  EXPECT_EQ(trades[0].getQuantity().value, 5);
  EXPECT_EQ(trades[1].getSellOrderId().value, 9002);
  EXPECT_EQ(trades[1].getQuantity().value, 1);
  ASSERT_TRUE(book.getBestAsk().has_value());
  EXPECT_EQ(book.getBestAsk()->value, 100);
}

TEST(OrderBookTest, RejectsOrderForDifferentSymbol) {
  OrderBook book{Symbol{"AAPL"}};

  EXPECT_THROW(
      book.addOrder(makeLimitOrder(1, 10001, Side::BUY, 10, 100, "MSFT")),
      std::invalid_argument);
}
