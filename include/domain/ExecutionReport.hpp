#pragma once

#include <optional>
#include <vector>

#include "domain/DomainTypeDefs.hpp"
#include "domain/Trade.hpp"

namespace trading::domain {

class ExecutionReport {
 public:
  ExecutionReport(OrderId orderId, ExecutionReportType type,
                  const Symbol& symbol, Side side, Type orderType,
                  Quantity originalQty, Quantity remainingQty,
                  std::optional<Price> price, Timestamp timestamp,
                  const std::vector<Trade>& trades = {});

  OrderId getOrderId() const noexcept;
  ExecutionReportType getType() const noexcept;
  const Symbol& getSymbol() const noexcept;
  Side getSide() const noexcept;
  Type getOrderType() const noexcept;
  Quantity getOriginalQuantity() const noexcept;
  Quantity getRemainingQuantity() const noexcept;
  std::optional<Price> getPrice() const noexcept;
  Timestamp getTimestamp() const noexcept;
  const std::vector<Trade>& getTrades() const noexcept;

 private:
  OrderId orderId_;
  ExecutionReportType type_;
  Symbol symbol_;
  Side side_;
  Type orderType_;
  Quantity originalQty_;
  Quantity remainingQty_;
  std::optional<Price> price_;
  Timestamp timestamp_;
  std::vector<Trade> trades_;
};

}  // namespace trading::domain
