#include <gtest/gtest.h>

#include "config/EngineConfig.hpp"

using namespace trading::config;

TEST(EngineConfigTest, DefaultBuilder) {
  auto config = EngineConfig::builder().build();

  EXPECT_EQ(config.getWorkerThreadCount(), 4);
  EXPECT_EQ(config.getQueueCapacity(), 10000);
  EXPECT_EQ(config.getPersistenceMode(), PersistenceMode::IN_MEMORY);
  EXPECT_EQ(config.getLogLevel(), LogLevel::INFO);
  EXPECT_TRUE(config.isMetricsEnabled());
  EXPECT_EQ(config.getMatchingStrategy(), MatchingStrategy::PRICE_TIME_PRIORITY);
  EXPECT_EQ(config.getRpcPort(), 50051);
  EXPECT_EQ(config.getDatabaseConnectionString(), "");
}

TEST(EngineConfigTest, BuilderChaining) {
  auto config = EngineConfig::builder()
                    .withWorkerThreadCount(8)
                    .withQueueCapacity(50000)
                    .withPersistenceMode(PersistenceMode::POSTGRESQL)
                    .withLogLevel(LogLevel::DEBUG)
                    .withMetricsEnabled(false)
                    .withMatchingStrategy(MatchingStrategy::FIFO)
                    .withRpcPort(9999)
                    .withDatabaseConnectionString("postgres://localhost:5432")
                    .build();

  EXPECT_EQ(config.getWorkerThreadCount(), 8);
  EXPECT_EQ(config.getQueueCapacity(), 50000);
  EXPECT_EQ(config.getPersistenceMode(), PersistenceMode::POSTGRESQL);
  EXPECT_EQ(config.getLogLevel(), LogLevel::DEBUG);
  EXPECT_FALSE(config.isMetricsEnabled());
  EXPECT_EQ(config.getMatchingStrategy(), MatchingStrategy::FIFO);
  EXPECT_EQ(config.getRpcPort(), 9999);
  EXPECT_EQ(config.getDatabaseConnectionString(), "postgres://localhost:5432");
}

TEST(EngineConfigTest, PartialBuilder) {
  auto config = EngineConfig::builder()
                    .withWorkerThreadCount(16)
                    .withLogLevel(LogLevel::ERROR)
                    .build();

  EXPECT_EQ(config.getWorkerThreadCount(), 16);
  EXPECT_EQ(config.getQueueCapacity(), 10000);
  EXPECT_EQ(config.getLogLevel(), LogLevel::ERROR);
  EXPECT_TRUE(config.isMetricsEnabled());
}

TEST(EngineConfigTest, MultipleBuilderInstances) {
  auto config1 = EngineConfig::builder()
                     .withWorkerThreadCount(4)
                     .withRpcPort(50051)
                     .build();

  auto config2 = EngineConfig::builder()
                     .withWorkerThreadCount(8)
                     .withRpcPort(50052)
                     .build();

  EXPECT_EQ(config1.getWorkerThreadCount(), 4);
  EXPECT_EQ(config1.getRpcPort(), 50051);
  EXPECT_EQ(config2.getWorkerThreadCount(), 8);
  EXPECT_EQ(config2.getRpcPort(), 50052);
}

TEST(EngineConfigTest, AllPersistenceModes) {
  auto inMemory = EngineConfig::builder()
                      .withPersistenceMode(PersistenceMode::IN_MEMORY)
                      .build();
  EXPECT_EQ(inMemory.getPersistenceMode(), PersistenceMode::IN_MEMORY);

  auto sqlite = EngineConfig::builder()
                    .withPersistenceMode(PersistenceMode::SQLITE)
                    .build();
  EXPECT_EQ(sqlite.getPersistenceMode(), PersistenceMode::SQLITE);

  auto postgresql = EngineConfig::builder()
                        .withPersistenceMode(PersistenceMode::POSTGRESQL)
                        .build();
  EXPECT_EQ(postgresql.getPersistenceMode(), PersistenceMode::POSTGRESQL);
}

TEST(EngineConfigTest, AllLogLevels) {
  auto debug = EngineConfig::builder().withLogLevel(LogLevel::DEBUG).build();
  EXPECT_EQ(debug.getLogLevel(), LogLevel::DEBUG);

  auto info = EngineConfig::builder().withLogLevel(LogLevel::INFO).build();
  EXPECT_EQ(info.getLogLevel(), LogLevel::INFO);

  auto warning = EngineConfig::builder().withLogLevel(LogLevel::WARNING).build();
  EXPECT_EQ(warning.getLogLevel(), LogLevel::WARNING);

  auto error = EngineConfig::builder().withLogLevel(LogLevel::ERROR).build();
  EXPECT_EQ(error.getLogLevel(), LogLevel::ERROR);
}

TEST(EngineConfigTest, AllMatchingStrategies) {
  auto priceTime = EngineConfig::builder()
                       .withMatchingStrategy(MatchingStrategy::PRICE_TIME_PRIORITY)
                       .build();
  EXPECT_EQ(priceTime.getMatchingStrategy(),
            MatchingStrategy::PRICE_TIME_PRIORITY);

  auto fifo = EngineConfig::builder()
                  .withMatchingStrategy(MatchingStrategy::FIFO)
                  .build();
  EXPECT_EQ(fifo.getMatchingStrategy(), MatchingStrategy::FIFO);
}

TEST(EngineConfigTest, LargeQueueCapacity) {
  auto config =
      EngineConfig::builder().withQueueCapacity(1000000000).build();
  EXPECT_EQ(config.getQueueCapacity(), 1000000000);
}

TEST(EngineConfigTest, BuilderIndependence) {
  auto builder1 = EngineConfig::builder().withWorkerThreadCount(2);
  auto builder2 = EngineConfig::builder().withWorkerThreadCount(4);

  auto config1 = builder1.build();
  auto config2 = builder2.build();

  EXPECT_EQ(config1.getWorkerThreadCount(), 2);
  EXPECT_EQ(config2.getWorkerThreadCount(), 4);
}
