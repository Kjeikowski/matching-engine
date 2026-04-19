#include <stdexcept>

#include "domain/Trade.hpp"

namespace trading::domain {

Trade::Trade(TradeId tradeId, OrderId buyId, OrderId sellId, Price price,
             Quantity qty, Timestamp ts)
    : tradeId_{tradeId},
      buyOrderId_{buyId},
      sellOrderId_{sellId},
      price_{price},
      quantity_{qty},
      timestamp_{ts} {
  validate(tradeId, buyId, sellId, price, qty, ts);
}

TradeId Trade::getTradeId() const noexcept { return tradeId_; }

OrderId Trade::getBuyOrderId() const noexcept { return buyOrderId_; }

OrderId Trade::getSellOrderId() const noexcept { return sellOrderId_; }

Price Trade::getPrice() const noexcept { return price_; }

Quantity Trade::getQuantity() const noexcept { return quantity_; }

Timestamp Trade::getTimestamp() const noexcept { return timestamp_; }

void Trade::validate(TradeId tradeId, OrderId buyId, OrderId sellId,
                     Price price, Quantity qty, Timestamp ts) {
  if (tradeId.value <= 0) {
    throw std::invalid_argument("TradeId must be greater than 0");
  }

  if (buyId.value <= 0) {
    throw std::invalid_argument("Buy order id must be greater than 0");
  }

  if (sellId.value <= 0) {
    throw std::invalid_argument("Sell order id must be greater than 0");
  }

  if (buyId.value == sellId.value) {
    throw std::invalid_argument("Buy and sell order ids must be different");
  }

  if (price.value <= 0) {
    throw std::invalid_argument("Trade price must be greater than 0");
  }

  if (qty.value <= 0) {
    throw std::invalid_argument("Trade quantity must be greater than 0");
  }

  if (ts.value <= 0) {
    throw std::invalid_argument("Timestamp must be greater than 0");
  }
}

}  // namespace trading::domain
