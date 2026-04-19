#pragma once

#include "domain/DomainTypeDefs.hpp"

namespace trading::domain {

class Trade {
 public:
  Trade(TradeId tradeId, OrderId buyId, OrderId sellId, Price price,
        Quantity qty, Timestamp ts);

  TradeId getTradeId() const noexcept;
  OrderId getBuyOrderId() const noexcept;
  OrderId getSellOrderId() const noexcept;
  Price getPrice() const noexcept;
  Quantity getQuantity() const noexcept;
  Timestamp getTimestamp() const noexcept;

 private:
  const TradeId tradeId_;
  const OrderId buyOrderId_;
  const OrderId sellOrderId_;
  const Price price_;
  const Quantity quantity_;
  const Timestamp timestamp_;

  static void validate(TradeId tradeId, OrderId buyId, OrderId sellId,
                       Price price, Quantity qty, Timestamp ts);
};

}  // namespace trading::domain
