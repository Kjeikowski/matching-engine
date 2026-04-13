#pragma once

#include <optional>
#include <stdexcept>

#include "domain/DomainTypeDefs.hpp"

namespace trading::domain {

class Order {
 public:
  Order(SequenceNumber sequenceNumber, OrderId orderId, const Symbol& symbol,
        Side side, Type type, Quantity quantity,
        std::optional<Price> price = std::nullopt);

  SequenceNumber getSequenceNumber() const noexcept;
  OrderId getOrderId() const noexcept;
  const Symbol& getSymbol() const noexcept;
  Side getSide() const noexcept;
  Type getType() const noexcept;
  Status getStatus() const noexcept;
  Quantity getOriginalQuantity() const noexcept;
  Quantity getRemainingQuantity() const noexcept;
  Quantity getFilledQuantity() const noexcept;
  std::optional<Price> getPrice() const noexcept;

  bool hasPrice() const noexcept;
  bool isActive() const noexcept;
  bool isFilled() const noexcept;
  bool isCanceled() const noexcept;

  void fill(Quantity executedQuantity);
  void cancel();

 private:
  static void validate(SequenceNumber sequenceNumber, OrderId orderId,
                       const Symbol& symbol, Type type, Quantity quantity,
                       std::optional<Price> price);

  SequenceNumber sequenceNumber_;
  OrderId orderId_;
  Symbol symbol_;
  Side side_;
  Type type_;
  Status status_;
  Quantity originalQuantity_;
  Quantity remainingQuantity_;
  std::optional<Price> price_;
};

}  // namespace trading::domain
