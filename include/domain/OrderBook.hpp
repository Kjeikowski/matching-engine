#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <optional>
#include <vector>

#include "domain/DomainTypeDefs.hpp"
#include "domain/Order.hpp"
#include "domain/Trade.hpp"

namespace trading::domain {

struct PriceAscending {
  bool operator()(const Price& lhs, const Price& rhs) const noexcept {
    return lhs.value < rhs.value;
  }
};

struct PriceDescending {
  bool operator()(const Price& lhs, const Price& rhs) const noexcept {
    return lhs.value > rhs.value;
  }
};

class OrderBook {
 public:
  explicit OrderBook(Symbol symbol);

  const Symbol& getSymbol() const noexcept;

  std::vector<Trade> addOrder(Order order);
  bool cancelOrder(OrderId orderId);

  std::optional<Price> getBestBid() const noexcept;
  std::optional<Price> getBestAsk() const noexcept;
  std::size_t getBidLevelCount() const noexcept;
  std::size_t getAskLevelCount() const noexcept;
  std::size_t getBidOrderCount() const noexcept;
  std::size_t getAskOrderCount() const noexcept;

 private:
  using BidLevels = std::map<Price, std::deque<Order>, PriceDescending>;
  using AskLevels = std::map<Price, std::deque<Order>, PriceAscending>;

  std::vector<Trade> matchBuyOrder(Order& incoming);
  std::vector<Trade> matchSellOrder(Order& incoming);

  bool canMatchBuy(const Order& incoming, Price bestAsk) const;
  bool canMatchSell(const Order& incoming, Price bestBid) const;

  void addRestingOrder(Order order);
  void validateOrderSymbol(const Order& order) const;
  Trade createTrade(const Order& incoming, const Order& resting,
                    Quantity executedQuantity);
  static std::size_t countOrders(const BidLevels& levels) noexcept;
  static std::size_t countOrders(const AskLevels& levels) noexcept;

  Symbol symbol_;
  BidLevels bids_;
  AskLevels asks_;
  TradeId nextTradeId_{1};
};

}  // namespace trading::domain
