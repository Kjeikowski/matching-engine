#include "domain/ExecutionReport.hpp"

namespace trading::domain {

ExecutionReport::ExecutionReport(OrderId orderId, ExecutionReportType type,
                                 const Symbol& symbol, Side side, Type orderType,
                                 Quantity originalQty, Quantity remainingQty,
                                 std::optional<Price> price, Timestamp timestamp,
                                 const std::vector<Trade>& trades)
    : orderId_(orderId),
      type_(type),
      symbol_(symbol),
      side_(side),
      orderType_(orderType),
      originalQty_(originalQty),
      remainingQty_(remainingQty),
      price_(price),
      timestamp_(timestamp),
      trades_(trades) {}

OrderId ExecutionReport::getOrderId() const noexcept { return orderId_; }

ExecutionReportType ExecutionReport::getType() const noexcept { return type_; }

const Symbol& ExecutionReport::getSymbol() const noexcept { return symbol_; }

Side ExecutionReport::getSide() const noexcept { return side_; }

Type ExecutionReport::getOrderType() const noexcept { return orderType_; }

Quantity ExecutionReport::getOriginalQuantity() const noexcept {
  return originalQty_;
}

Quantity ExecutionReport::getRemainingQuantity() const noexcept {
  return remainingQty_;
}

std::optional<Price> ExecutionReport::getPrice() const noexcept { return price_; }

Timestamp ExecutionReport::getTimestamp() const noexcept { return timestamp_; }

const std::vector<Trade>& ExecutionReport::getTrades() const noexcept {
  return trades_;
}

}  // namespace trading::domain
