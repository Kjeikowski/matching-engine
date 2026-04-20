#include <gtest/gtest.h>

#include "domain/ExecutionReport.hpp"
#include "domain/Order.hpp"
#include "domain/Trade.hpp"

using namespace trading::domain;

namespace {

Order makeLimitOrder(std::int64_t seq, std::int64_t id, const Symbol& symbol,
                     Side side, std::int64_t qty, std::int64_t price) {
  return Order{SequenceNumber{seq}, OrderId{id},   symbol, side,
               Type::LIMIT,         Quantity{qty}, Price{price}};
}

Trade makeTrade(std::int64_t tradeId, std::int64_t buyId, std::int64_t sellId,
                std::int64_t price, std::int64_t qty) {
  return Trade{TradeId{tradeId}, OrderId{buyId}, OrderId{sellId}, Price{price},
               Quantity{qty}, Timestamp{1234567890}};
}

}  // namespace

TEST(ExecutionReportTest, NewOrderReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::NEW, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{100},
                         Price{150}, timestamp};

  EXPECT_EQ(report.getOrderId().value, 1001);
  EXPECT_EQ(report.getType(), ExecutionReportType::NEW);
  EXPECT_EQ(report.getSymbol().value, "AAPL");
  EXPECT_EQ(report.getSide(), Side::BUY);
  EXPECT_EQ(report.getOrderType(), Type::LIMIT);
  EXPECT_EQ(report.getOriginalQuantity().value, 100);
  EXPECT_EQ(report.getRemainingQuantity().value, 100);
  ASSERT_TRUE(report.getPrice().has_value());
  EXPECT_EQ(report.getPrice()->value, 150);
  EXPECT_EQ(report.getTimestamp().value, 1234567890);
  EXPECT_TRUE(report.getTrades().empty());
}

TEST(ExecutionReportTest, FillReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {makeTrade(1, 1001, 2001, 150, 100)};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::FILL, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{0},
                         Price{150}, timestamp, trades};

  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  ASSERT_EQ(report.getTrades().size(), 1U);
  EXPECT_EQ(report.getTrades()[0].getTradeId().value, 1);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 100);
}

TEST(ExecutionReportTest, PartialFillReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {makeTrade(1, 1001, 2001, 150, 50)};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::PARTIAL_FILL, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{50},
                         Price{150}, timestamp, trades};

  EXPECT_EQ(report.getType(), ExecutionReportType::PARTIAL_FILL);
  EXPECT_EQ(report.getOriginalQuantity().value, 100);
  EXPECT_EQ(report.getRemainingQuantity().value, 50);
  ASSERT_EQ(report.getTrades().size(), 1U);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 50);
}

TEST(ExecutionReportTest, CancelReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::CANCEL, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{100},
                         Price{150}, timestamp};

  EXPECT_EQ(report.getType(), ExecutionReportType::CANCEL);
  EXPECT_EQ(report.getRemainingQuantity().value, 100);
  EXPECT_TRUE(report.getTrades().empty());
}

TEST(ExecutionReportTest, RejectReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::REJECT, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{100},
                         Price{150}, timestamp};

  EXPECT_EQ(report.getType(), ExecutionReportType::REJECT);
  EXPECT_TRUE(report.getTrades().empty());
}

TEST(ExecutionReportTest, MarketOrderReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {makeTrade(1, 1001, 2001, 150, 100)};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::FILL, symbol,
                         Side::BUY, Type::MARKET, Quantity{100}, Quantity{0},
                         std::nullopt, timestamp, trades};

  EXPECT_EQ(report.getOrderType(), Type::MARKET);
  EXPECT_FALSE(report.getPrice().has_value());
  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
}

TEST(ExecutionReportTest, MultipleTradesInReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {
      makeTrade(1, 1001, 2001, 150, 50),
      makeTrade(2, 1001, 2002, 151, 50)
  };

  ExecutionReport report{OrderId{1001}, ExecutionReportType::FILL, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{0},
                         Price{151}, timestamp, trades};

  ASSERT_EQ(report.getTrades().size(), 2U);
  EXPECT_EQ(report.getTrades()[0].getQuantity().value, 50);
  EXPECT_EQ(report.getTrades()[1].getQuantity().value, 50);
  EXPECT_EQ(report.getTrades()[0].getPrice().value, 150);
  EXPECT_EQ(report.getTrades()[1].getPrice().value, 151);
}

TEST(ExecutionReportTest, SellOrderReport) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {makeTrade(1, 1001, 2001, 150, 100)};

  ExecutionReport report{OrderId{2001}, ExecutionReportType::FILL, symbol,
                         Side::SELL, Type::LIMIT, Quantity{100}, Quantity{0},
                         Price{150}, timestamp, trades};

  EXPECT_EQ(report.getSide(), Side::SELL);
  EXPECT_EQ(report.getOrderId().value, 2001);
  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
}

TEST(ExecutionReportTest, EmptyTradesVector) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> emptyTrades;

  ExecutionReport report{OrderId{1001}, ExecutionReportType::NEW, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{100},
                         Price{150}, timestamp, emptyTrades};

  EXPECT_TRUE(report.getTrades().empty());
}

TEST(ExecutionReportTest, DefaultTradesParameter) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::NEW, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{100},
                         Price{150}, timestamp};

  EXPECT_TRUE(report.getTrades().empty());
}

TEST(ExecutionReportTest, AllExecutionReportTypes) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  std::vector<ExecutionReportType> types = {
      ExecutionReportType::NEW,
      ExecutionReportType::PARTIAL_FILL,
      ExecutionReportType::FILL,
      ExecutionReportType::CANCEL,
      ExecutionReportType::REJECT
  };

  for (auto type : types) {
    ExecutionReport report{OrderId{1001}, type, symbol, Side::BUY, Type::LIMIT,
                           Quantity{100}, Quantity{100}, Price{150}, timestamp};
    EXPECT_EQ(report.getType(), type);
  }
}

TEST(ExecutionReportTest, LargeQuantities) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::NEW, symbol,
                         Side::BUY, Type::LIMIT, Quantity{1000000}, Quantity{1000000},
                         Price{150}, timestamp};

  EXPECT_EQ(report.getOriginalQuantity().value, 1000000);
  EXPECT_EQ(report.getRemainingQuantity().value, 1000000);
}

TEST(ExecutionReportTest, ZeroRemainingQuantity) {
  Symbol symbol{"AAPL"};
  Timestamp timestamp{1234567890};
  std::vector<Trade> trades = {makeTrade(1, 1001, 2001, 150, 100)};

  ExecutionReport report{OrderId{1001}, ExecutionReportType::FILL, symbol,
                         Side::BUY, Type::LIMIT, Quantity{100}, Quantity{0},
                         Price{150}, timestamp, trades};

  EXPECT_EQ(report.getRemainingQuantity().value, 0);
  EXPECT_EQ(report.getType(), ExecutionReportType::FILL);
}
