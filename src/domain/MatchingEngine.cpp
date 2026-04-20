#include "domain/MatchingEngine.hpp"

#include <algorithm>
#include <stdexcept>

namespace trading::domain {

MatchingEngine::MatchingEngine() = default;

ExecutionReport MatchingEngine::submitOrder(Order order) {
  Symbol symbol = order.getSymbol();
  const OrderId orderId = order.getOrderId();
  const Quantity originalQty = order.getOriginalQuantity();
  const Side side = order.getSide();
  const Type orderType = order.getType();
  const std::optional<Price> price = order.getPrice();

  auto [it, inserted] = orderToSymbolMap_.insert({orderId, symbol});
  if (!inserted) {
    throw std::invalid_argument("Order ID already exists");
  }

  OrderBook& book = getOrCreateOrderBook(symbol);

  std::vector<Trade> trades = book.addOrder(std::move(order));

  for (const auto& trade : trades) {
    totalTrades_++;
    totalVolume_ += trade.getQuantity().value;
  }

  auto orderBookOpt = getOrderBook(symbol);
  const OrderBook* orderBook = orderBookOpt ? *orderBookOpt : nullptr;
  std::optional<Price> bestBid = orderBook ? orderBook->getBestBid() : std::nullopt;
  std::optional<Price> bestAsk = orderBook ? orderBook->getBestAsk() : std::nullopt;

  Timestamp timestamp{1234567890};

  ExecutionReportType reportType;
  Quantity remainingQty{0};

  if (trades.empty()) {
    reportType = ExecutionReportType::NEW;
    remainingQty = originalQty;
  } else {
    Quantity filledQty{0};
    for (const auto& trade : trades) {
      filledQty.value += trade.getQuantity().value;
    }
    
    if (filledQty.value == originalQty.value) {
      reportType = ExecutionReportType::FILL;
      remainingQty = Quantity{0};
    } else {
      reportType = ExecutionReportType::PARTIAL_FILL;
      remainingQty = Quantity{originalQty.value - filledQty.value};
    }
  }

  return ExecutionReport{orderId, reportType, symbol, side, orderType,
                         originalQty, remainingQty, price, timestamp, trades};
}

bool MatchingEngine::cancelOrder(OrderId orderId) {
  auto symbolIt = orderToSymbolMap_.find(orderId);
  if (symbolIt == orderToSymbolMap_.end()) {
    return false;
  }

  const Symbol& symbol = symbolIt->second;

  auto bookIt = orderBooks_.find(symbol.value);
  if (bookIt == orderBooks_.end()) {
    return false;
  }

  if (bookIt->second.cancelOrder(orderId)) {
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
