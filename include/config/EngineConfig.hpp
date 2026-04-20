#pragma once

#include <cstdint>
#include <string>

namespace trading::config {

enum class PersistenceMode { IN_MEMORY, SQLITE, POSTGRESQL };

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

enum class MatchingStrategy { PRICE_TIME_PRIORITY, FIFO };

class EngineConfig {
 public:
  struct Builder {
    Builder& withWorkerThreadCount(std::int32_t count) noexcept;
    Builder& withQueueCapacity(std::int64_t capacity) noexcept;
    Builder& withPersistenceMode(PersistenceMode mode) noexcept;
    Builder& withLogLevel(LogLevel level) noexcept;
    Builder& withMetricsEnabled(bool enabled) noexcept;
    Builder& withMatchingStrategy(MatchingStrategy strategy) noexcept;
    Builder& withRpcPort(std::int32_t port) noexcept;
    Builder& withDatabaseConnectionString(const std::string& connStr) noexcept;

    EngineConfig build() const;

    std::int32_t workerThreadCount{4};
    std::int64_t queueCapacity{10000};
    PersistenceMode persistenceMode{PersistenceMode::IN_MEMORY};
    LogLevel logLevel{LogLevel::INFO};
    bool metricsEnabled{true};
    MatchingStrategy matchingStrategy{MatchingStrategy::PRICE_TIME_PRIORITY};
    std::int32_t rpcPort{50051};
    std::string databaseConnectionString{};
  };

  EngineConfig(const Builder& builder);

  std::int32_t getWorkerThreadCount() const noexcept;
  std::int64_t getQueueCapacity() const noexcept;
  PersistenceMode getPersistenceMode() const noexcept;
  LogLevel getLogLevel() const noexcept;
  bool isMetricsEnabled() const noexcept;
  MatchingStrategy getMatchingStrategy() const noexcept;
  std::int32_t getRpcPort() const noexcept;
  const std::string& getDatabaseConnectionString() const noexcept;

  static Builder builder() noexcept;

 private:
  std::int32_t workerThreadCount_;
  std::int64_t queueCapacity_;
  PersistenceMode persistenceMode_;
  LogLevel logLevel_;
  bool metricsEnabled_;
  MatchingStrategy matchingStrategy_;
  std::int32_t rpcPort_;
  std::string databaseConnectionString_;
};

}  // namespace trading::config
