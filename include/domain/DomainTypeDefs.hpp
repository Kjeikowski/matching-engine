#pragma once

#include <cstdint>
#include <string>

namespace trading::domain {

struct TradeId {
  explicit TradeId(std::int64_t v) : value(v) {}
  std::int64_t value;
};

struct Symbol {
  explicit Symbol(std::string v) : value(v) {}
  std::string value;
  
  bool operator<(const Symbol& other) const noexcept {
    return value < other.value;
  }
};

struct SequenceNumber {
  explicit SequenceNumber(std::int64_t v) : value(v) {}
  std::int64_t value;
};

struct OrderId {
  explicit OrderId(std::int64_t v) : value(v) {}
  std::int64_t value;
  
  bool operator<(const OrderId& other) const noexcept {
    return value < other.value;
  }
};

struct Quantity {
  explicit Quantity(std::int64_t v) : value(v) {}
  std::int64_t value;
};

struct Timestamp {
  explicit Timestamp(std::int64_t v) : value(v) {}
  std::int64_t value;  // microseconds since Unix epoch
};

struct Price {
  explicit Price(std::int64_t v) : value(v) {}
  std::int64_t value;
};

enum class Side { BUY, SELL };

enum class Type { MARKET, LIMIT };

enum class Status { NEW, PARTIALLY_FILLED, FILLED, CANCELED };

}  // namespace trading::domain
