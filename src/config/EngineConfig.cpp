#include "config/EngineConfig.hpp"

namespace trading::config {

EngineConfig::Builder& EngineConfig::Builder::withWorkerThreadCount(
    std::int32_t count) noexcept {
  workerThreadCount = count;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withQueueCapacity(
    std::int64_t capacity) noexcept {
  queueCapacity = capacity;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withPersistenceMode(
    PersistenceMode mode) noexcept {
  persistenceMode = mode;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withLogLevel(
    LogLevel level) noexcept {
  logLevel = level;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withMetricsEnabled(
    bool enabled) noexcept {
  metricsEnabled = enabled;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withMatchingStrategy(
    MatchingStrategy strategy) noexcept {
  matchingStrategy = strategy;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withRpcPort(
    std::int32_t port) noexcept {
  rpcPort = port;
  return *this;
}

EngineConfig::Builder& EngineConfig::Builder::withDatabaseConnectionString(
    const std::string& connStr) noexcept {
  databaseConnectionString = connStr;
  return *this;
}

EngineConfig EngineConfig::Builder::build() const {
  return EngineConfig(*this);
}

EngineConfig::EngineConfig(const Builder& builder)
    : workerThreadCount_(builder.workerThreadCount),
      queueCapacity_(builder.queueCapacity),
      persistenceMode_(builder.persistenceMode),
      logLevel_(builder.logLevel),
      metricsEnabled_(builder.metricsEnabled),
      matchingStrategy_(builder.matchingStrategy),
      rpcPort_(builder.rpcPort),
      databaseConnectionString_(builder.databaseConnectionString) {}

std::int32_t EngineConfig::getWorkerThreadCount() const noexcept {
  return workerThreadCount_;
}

std::int64_t EngineConfig::getQueueCapacity() const noexcept {
  return queueCapacity_;
}

PersistenceMode EngineConfig::getPersistenceMode() const noexcept {
  return persistenceMode_;
}

LogLevel EngineConfig::getLogLevel() const noexcept { return logLevel_; }

bool EngineConfig::isMetricsEnabled() const noexcept {
  return metricsEnabled_;
}

MatchingStrategy EngineConfig::getMatchingStrategy() const noexcept {
  return matchingStrategy_;
}

std::int32_t EngineConfig::getRpcPort() const noexcept { return rpcPort_; }

const std::string& EngineConfig::getDatabaseConnectionString() const noexcept {
  return databaseConnectionString_;
}

EngineConfig::Builder EngineConfig::builder() noexcept { return Builder{}; }

}  // namespace trading::config
