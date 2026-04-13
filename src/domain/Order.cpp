#include <stdexcept>

#include "domain/Order.hpp"

namespace trading::domain {

Order::Order(SequenceNumber sequenceNumber, OrderId orderId,
             const Symbol& symbol, Side side, Type type, Quantity quantity,
             std::optional<Price> price)
    : sequenceNumber_(sequenceNumber),
      orderId_(orderId),
      symbol_(symbol),
      side_(side),
      type_(type),
      status_(Status::NEW),
      originalQuantity_(quantity),
      remainingQuantity_(quantity),
      price_(price) {
  validate(sequenceNumber, orderId, symbol, type, quantity, price);
}

SequenceNumber Order::getSequenceNumber() const noexcept {
  return sequenceNumber_;
}

OrderId Order::getOrderId() const noexcept { return orderId_; }

const Symbol& Order::getSymbol() const noexcept { return symbol_; }

Side Order::getSide() const noexcept { return side_; }

Type Order::getType() const noexcept { return type_; }

Status Order::getStatus() const noexcept { return status_; }

Quantity Order::getOriginalQuantity() const noexcept {
  return originalQuantity_;
}

Quantity Order::getRemainingQuantity() const noexcept {
  return remainingQuantity_;
}

Quantity Order::getFilledQuantity() const noexcept {
  return Quantity{originalQuantity_.value - remainingQuantity_.value};
}

std::optional<Price> Order::getPrice() const noexcept { return price_; }

bool Order::hasPrice() const noexcept { return price_.has_value(); }

bool Order::isActive() const noexcept {
  return status_ == Status::NEW || status_ == Status::PARTIALLY_FILLED;
}

bool Order::isFilled() const noexcept { return status_ == Status::FILLED; }

bool Order::isCanceled() const noexcept { return status_ == Status::CANCELED; }

void Order::fill(Quantity executedQuantity) {
  if (status_ == Status::CANCELED) {
    throw std::logic_error("Cannot fill a canceled order");
  }

  if (status_ == Status::FILLED) {
    throw std::logic_error("Cannot fill an already filled order");
  }

  if (executedQuantity.value <= 0) {
    throw std::invalid_argument("Executed quantity must be > 0");
  }

  if (executedQuantity.value > remainingQuantity_.value) {
    throw std::invalid_argument(
        "Executed quantity cannot exceed remaining quantity");
  }

  remainingQuantity_ =
      Quantity{remainingQuantity_.value - executedQuantity.value};

  if (remainingQuantity_.value == 0) {
    status_ = Status::FILLED;
  } else {
    status_ = Status::PARTIALLY_FILLED;
  }
}

void Order::cancel() {
  if (status_ == Status::FILLED) {
    throw std::logic_error("Cannot cancel a filled order");
  }

  if (status_ == Status::CANCELED) {
    throw std::logic_error("Order is already canceled");
  }

  status_ = Status::CANCELED;
}

void Order::validate(SequenceNumber sequenceNumber, OrderId orderId,
                     const Symbol& symbol, Type type, Quantity quantity,
                     std::optional<Price> price) {
  if (sequenceNumber.value < 0) {
    throw std::invalid_argument("Sequence number must be >= 0");
  }

  if (orderId.value <= 0) {
    throw std::invalid_argument("Order ID must be > 0");
  }

  if (symbol.value.empty()) {
    throw std::invalid_argument("Symbol must not be empty");
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
    throw std::invalid_argument("Market order must not have a price");
  }
}

}  // namespace trading::domain
