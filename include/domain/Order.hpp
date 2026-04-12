#pragma once

#include <optional>
#include <stdexcept>

#include "domain/DomainTypeDefs.hpp"

namespace trading::domain {

class Order {
 private:
  SequenceNumber sequenceNumber;
  OrderId orderId;

  Quantity originalQuantity;
  Quantity remainingQuantity;

  std::optional<Price> price;

  Side side;
  Type type;
  Status status;

  Symbol symbol;

 public:
  Order(SequenceNumber seqNum,
        OrderId ordId,
        const Symbol& sym,
        Side s,
        Type t,
        Quantity quantity,
        std::optional<Price> p);

  SequenceNumber getSequenceNumber() const;
  OrderId getOrderId() const;
  const Symbol& getSymbol() const;
  Side getSide() const;
  Type getType() const;
  Status getStatus() const;
  Quantity getOriginalQuantity() const;
  Quantity getRemainingQuantity() const;
  std::optional<Price> getPrice() const;
};

}  // namespace trading::domain
