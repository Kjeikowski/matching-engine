#include "domain/MatchingEngine.hpp"

#include <algorithm>
#include <stdexcept>

namespace trading::domain {

MatchingEngine::MatchingEngine() = default;

std::vector<Trade> MatchingEngine::submitOrder(Order order) {
  const Symbol& symbol = order.getSymbol();
  const OrderId orderId = order.getOrderId();

  // Add to order-to-symbol map for tracking
  auto [it, inserted] = orderToSymbolMap_.insert({orderId, symbol});
  if (!inserted) {
    throw std::invalid_argument("Order ID already exists");
  }

  // Get or create the order book for this symbol
  OrderBook& book = getOrCreateOrderBook(symbol);

  // Process the order and collect trades
  std::vector<Trade> trades = book.addOrder(std::move(order));

  // Track metrics
  for (const auto& trade : trades) {
    totalTrades_++;
    totalVolume_ += trade.getQuantity().value;
  }

  return trades;
}

bool MatchingEngine::cancelOrder(OrderId orderId) {
  // Find the symbol for this order
  auto symbolIt = orderToSymbolMap_.find(orderId);
  if (symbolIt == orderToSymbolMap_.end()) {
    return false;  // Order not found
  }

  const Symbol& symbol = symbolIt->second;

  // Get the order book (should exist since we have it in the map)
  auto bookIt = orderBooks_.find(symbol.value);
  if (bookIt == orderBooks_.end()) {
    return false;  // Order book disappeared (shouldn't happen)
  }

  // Try to cancel the order
  if (bookIt->second.cancelOrder(orderId)) {
    // Remove from tracking map
    orderToSymbolMap_.erase(symbolIt);
    return true;
  }

  return false;
}

std::optional<OrderBook*> MatchingEngine::getOrderBook(
    const Symbol& symbol) noexcept {
  auto it = orderBooks_.find(symbol.value);
  if (it != orderBooks_.end()) {
    return &it->second;
  }
  return std::nullopt;
}

std::optional<const OrderBook*> MatchingEngine::getOrderBook(
    const Symbol& symbol) const noexcept {
  auto it = orderBooks_.find(symbol.value);
  if (it != orderBooks_.end()) {
    return &it->second;
  }
  return std::nullopt;
}

bool MatchingEngine::hasOrderBook(const Symbol& symbol) const noexcept {
  return orderBooks_.find(symbol.value) != orderBooks_.end();
}

std::size_t MatchingEngine::getTotalOrderBookCount() const noexcept {
  return orderBooks_.size();
}

std::size_t MatchingEngine::getTotalTradeCount() const noexcept {
  return totalTrades_;
}

std::int64_t MatchingEngine::getTotalTradeVolume() const noexcept {
  return totalVolume_;
}

OrderBook& MatchingEngine::getOrCreateOrderBook(const Symbol& symbol) {
  auto [it, inserted] = orderBooks_.insert({symbol.value, OrderBook{symbol}});
  return it->second;
}

}  // namespace trading::domain
