#pragma once

#include <map>
#include <optional>
#include <vector>

#include "domain/DomainTypeDefs.hpp"
#include "domain/Order.hpp"
#include "domain/OrderBook.hpp"
#include "domain/Trade.hpp"

namespace trading::domain {

class MatchingEngine {
 public:
  MatchingEngine();

  // Order operations
  std::vector<Trade> submitOrder(Order order);
  bool cancelOrder(OrderId orderId);

  // Query operations
  std::optional<OrderBook*> getOrderBook(const Symbol& symbol) noexcept;
  std::optional<const OrderBook*> getOrderBook(
      const Symbol& symbol) const noexcept;
  bool hasOrderBook(const Symbol& symbol) const noexcept;

  // Metrics
  std::size_t getTotalOrderBookCount() const noexcept;
  std::size_t getTotalTradeCount() const noexcept;
  std::int64_t getTotalTradeVolume() const noexcept;

 private:
  // Internal state management
  OrderBook& getOrCreateOrderBook(const Symbol& symbol);

  // State
  std::map<std::string, OrderBook> orderBooks_;
  std::map<OrderId, Symbol> orderToSymbolMap_;
  std::size_t totalTrades_{0};
  std::int64_t totalVolume_{0};
};

}  // namespace trading::domain
