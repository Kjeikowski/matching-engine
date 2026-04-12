#include "domain/Order.hpp"

namespace trading::domain {

Order::Order(SequenceNumber seqNum, OrderId ordId, const Symbol& sym, Side s,
             Type t, Quantity quantity, std::optional<Price> p)
    : sequenceNumber(seqNum),
      orderId(ordId),
      originalQuantity(quantity),
      remainingQuantity(quantity),
      price(p),
      side(s),
      type(t),
      status(Status::NEW),
      symbol(sym) {
  if (seqNum.value < 0) {
    throw std::invalid_argument("Sequence number must be >= 0");
  }

  if (ordId.value <= 0) {
    throw std::invalid_argument("Order ID must be > 0");
  }

  if (quantity.value <= 0) {
    throw std::invalid_argument("Quantity must be > 0");
  }

  if (type == Type::LIMIT) {
    if (!price.has_value()) {
      throw std::invalid_argument("Limit order must have a price");
    }
    if (price->value <= 0) {
      throw std::invalid_argument("Price must be > 0");
    }
  }

  if (type == Type::MARKET && price.has_value()) {
    throw std::invalid_argument("Market order must NOT have a price");
  }
}

SequenceNumber Order::getSequenceNumber() const { return sequenceNumber; }
OrderId Order::getOrderId() const { return orderId; }
const Symbol& Order::getSymbol() const { return symbol; }
Side Order::getSide() const { return side; }
Type Order::getType() const { return type; }
Status Order::getStatus() const { return status; }
Quantity Order::getOriginalQuantity() const { return originalQuantity; }
Quantity Order::getRemainingQuantity() const { return remainingQuantity; }
std::optional<Price> Order::getPrice() const { return price; }

}  // namespace trading::domain
