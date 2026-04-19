#include <algorithm>
#include <stdexcept>

#include "domain/OrderBook.hpp"

namespace trading::domain {

OrderBook::OrderBook(Symbol symbol) : symbol_(std::move(symbol)) {
  if (symbol_.value.empty()) {
    throw std::invalid_argument("OrderBook symbol must not be empty");
  }
}

const Symbol& OrderBook::getSymbol() const noexcept { return symbol_; }

std::vector<Trade> OrderBook::addOrder(Order order) {
  validateOrderSymbol(order);

  std::vector<Trade> trades;
  if (order.getSide() == Side::BUY) {
    trades = matchBuyOrder(order);
  } else {
    trades = matchSellOrder(order);
  }

  if (order.isActive() && order.getType() == Type::LIMIT) {
    addRestingOrder(std::move(order));
  }

  return trades;
}

bool OrderBook::cancelOrder(OrderId orderId) {
  for (auto levelIt = bids_.begin(); levelIt != bids_.end(); ++levelIt) {
    auto& orders = levelIt->second;
    for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
      if (orderIt->getOrderId().value == orderId.value) {
        orderIt->cancel();
        orders.erase(orderIt);
        if (orders.empty()) {
          bids_.erase(levelIt);
        }
        return true;
      }
    }
  }

  for (auto levelIt = asks_.begin(); levelIt != asks_.end(); ++levelIt) {
    auto& orders = levelIt->second;
    for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
      if (orderIt->getOrderId().value == orderId.value) {
        orderIt->cancel();
        orders.erase(orderIt);
        if (orders.empty()) {
          asks_.erase(levelIt);
        }
        return true;
      }
    }
  }

  return false;
}

std::optional<Price> OrderBook::getBestBid() const noexcept {
  if (bids_.empty()) {
    return std::nullopt;
  }
  return bids_.begin()->first;
}

std::optional<Price> OrderBook::getBestAsk() const noexcept {
  if (asks_.empty()) {
    return std::nullopt;
  }
  return asks_.begin()->first;
}

std::size_t OrderBook::getBidLevelCount() const noexcept {
  return bids_.size();
}

std::size_t OrderBook::getAskLevelCount() const noexcept {
  return asks_.size();
}

std::size_t OrderBook::getBidOrderCount() const noexcept {
  return countOrders(bids_);
}

std::size_t OrderBook::getAskOrderCount() const noexcept {
  return countOrders(asks_);
}

std::vector<Trade> OrderBook::matchBuyOrder(Order& incoming) {
  std::vector<Trade> trades;

  while (incoming.isActive() && !asks_.empty()) {
    auto bestAskIt = asks_.begin();
    if (!canMatchBuy(incoming, bestAskIt->first)) {
      break;
    }

    auto& restingOrders = bestAskIt->second;
    auto& resting = restingOrders.front();

    const Quantity executedQuantity{
        std::min(incoming.getRemainingQuantity().value,
                 resting.getRemainingQuantity().value)};

    trades.push_back(createTrade(incoming, resting, executedQuantity));
    incoming.fill(executedQuantity);
    resting.fill(executedQuantity);

    if (resting.isFilled()) {
      restingOrders.pop_front();
    }
    if (restingOrders.empty()) {
      asks_.erase(bestAskIt);
    }
  }

  return trades;
}

std::vector<Trade> OrderBook::matchSellOrder(Order& incoming) {
  std::vector<Trade> trades;

  while (incoming.isActive() && !bids_.empty()) {
    auto bestBidIt = bids_.begin();
    if (!canMatchSell(incoming, bestBidIt->first)) {
      break;
    }

    auto& restingOrders = bestBidIt->second;
    auto& resting = restingOrders.front();

    const Quantity executedQuantity{
        std::min(incoming.getRemainingQuantity().value,
                 resting.getRemainingQuantity().value)};

    trades.push_back(createTrade(incoming, resting, executedQuantity));
    incoming.fill(executedQuantity);
    resting.fill(executedQuantity);

    if (resting.isFilled()) {
      restingOrders.pop_front();
    }
    if (restingOrders.empty()) {
      bids_.erase(bestBidIt);
    }
  }

  return trades;
}

bool OrderBook::canMatchBuy(const Order& incoming, Price bestAsk) const {
  if (incoming.getType() == Type::MARKET) {
    return true;
  }
  return incoming.getPrice()->value >= bestAsk.value;
}

bool OrderBook::canMatchSell(const Order& incoming, Price bestBid) const {
  if (incoming.getType() == Type::MARKET) {
    return true;
  }
  return incoming.getPrice()->value <= bestBid.value;
}

void OrderBook::addRestingOrder(Order order) {
  if (!order.isActive()) {
    return;
  }

  if (order.getType() != Type::LIMIT) {
    throw std::logic_error("Only active limit orders may rest in the book");
  }

  if (order.getSide() == Side::BUY) {
    bids_[*order.getPrice()].push_back(std::move(order));
  } else {
    asks_[*order.getPrice()].push_back(std::move(order));
  }
}

void OrderBook::validateOrderSymbol(const Order& order) const {
  if (order.getSymbol().value != symbol_.value) {
    throw std::invalid_argument("Order symbol does not match OrderBook symbol");
  }
}

Trade OrderBook::createTrade(const Order& incoming, const Order& resting,
                             Quantity executedQuantity) {
  const bool incomingIsBuy = incoming.getSide() == Side::BUY;
  const OrderId buyId =
      incomingIsBuy ? incoming.getOrderId() : resting.getOrderId();
  const OrderId sellId =
      incomingIsBuy ? resting.getOrderId() : incoming.getOrderId();
  const Price tradePrice = *resting.getPrice();
  const Timestamp timestamp{std::max(incoming.getSequenceNumber().value,
                                     resting.getSequenceNumber().value)};

  const Trade trade{nextTradeId_,     buyId,    sellId, tradePrice,
                    executedQuantity, timestamp};
  nextTradeId_ = TradeId{nextTradeId_.value + 1};
  return trade;
}

std::size_t OrderBook::countOrders(const BidLevels& levels) noexcept {
  std::size_t count = 0;
  for (const auto& [price, orders] : levels) {
    (void)price;
    count += orders.size();
  }
  return count;
}

std::size_t OrderBook::countOrders(const AskLevels& levels) noexcept {
  std::size_t count = 0;
  for (const auto& [price, orders] : levels) {
    (void)price;
    count += orders.size();
  }
  return count;
}

}  // namespace trading::domain
