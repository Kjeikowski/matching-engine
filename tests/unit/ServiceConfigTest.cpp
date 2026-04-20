#include <gtest/gtest.h>

#include "config/ServiceConfig.hpp"

using namespace trading::config;

TEST(ServiceConfigTest, DefaultBuilder) {
  auto config = ServiceConfig::builder().build();

  EXPECT_EQ(config.getServiceName(), "MatchingService");
  EXPECT_EQ(config.getPort(), 50051);
  EXPECT_EQ(config.getHost(), "0.0.0.0");
  EXPECT_EQ(config.getMaxConnections(), 100);
  EXPECT_EQ(config.getRequestTimeoutMs(), 5000);
  EXPECT_FALSE(config.isSSLEnabled());
  EXPECT_EQ(config.getCertPath(), "");
  EXPECT_EQ(config.getKeyPath(), "");
}

TEST(ServiceConfigTest, BuilderChaining) {
  auto config = ServiceConfig::builder()
                    .withServiceName("OrderService")
                    .withPort(9000)
                    .withHost("127.0.0.1")
                    .withMaxConnections(500)
                    .withRequestTimeoutMs(10000)
                    .withEnableSSL(true)
                    .withCertPath("/etc/ssl/certs/server.crt")
                    .withKeyPath("/etc/ssl/private/server.key")
                    .build();

  EXPECT_EQ(config.getServiceName(), "OrderService");
  EXPECT_EQ(config.getPort(), 9000);
  EXPECT_EQ(config.getHost(), "127.0.0.1");
  EXPECT_EQ(config.getMaxConnections(), 500);
  EXPECT_EQ(config.getRequestTimeoutMs(), 10000);
  EXPECT_TRUE(config.isSSLEnabled());
  EXPECT_EQ(config.getCertPath(), "/etc/ssl/certs/server.crt");
  EXPECT_EQ(config.getKeyPath(), "/etc/ssl/private/server.key");
}

TEST(ServiceConfigTest, PartialBuilder) {
  auto config = ServiceConfig::builder()
                    .withServiceName("TradeService")
                    .withPort(9001)
                    .build();

  EXPECT_EQ(config.getServiceName(), "TradeService");
  EXPECT_EQ(config.getPort(), 9001);
  EXPECT_EQ(config.getHost(), "0.0.0.0");
  EXPECT_EQ(config.getMaxConnections(), 100);
  EXPECT_FALSE(config.isSSLEnabled());
}

TEST(ServiceConfigTest, MultipleBuilderInstances) {
  auto config1 = ServiceConfig::builder()
                     .withServiceName("Service1")
                     .withPort(9000)
                     .build();

  auto config2 = ServiceConfig::builder()
                     .withServiceName("Service2")
                     .withPort(9001)
                     .build();

  EXPECT_EQ(config1.getServiceName(), "Service1");
  EXPECT_EQ(config1.getPort(), 9000);
  EXPECT_EQ(config2.getServiceName(), "Service2");
  EXPECT_EQ(config2.getPort(), 9001);
}

TEST(ServiceConfigTest, SSLConfiguration) {
  auto noSSL = ServiceConfig::builder().withEnableSSL(false).build();
  EXPECT_FALSE(noSSL.isSSLEnabled());
  EXPECT_EQ(noSSL.getCertPath(), "");
  EXPECT_EQ(noSSL.getKeyPath(), "");

  auto withSSL = ServiceConfig::builder()
                     .withEnableSSL(true)
                     .withCertPath("/path/to/cert")
                     .withKeyPath("/path/to/key")
                     .build();
  EXPECT_TRUE(withSSL.isSSLEnabled());
  EXPECT_EQ(withSSL.getCertPath(), "/path/to/cert");
  EXPECT_EQ(withSSL.getKeyPath(), "/path/to/key");
}

TEST(ServiceConfigTest, ConnectionLimits) {
  auto lowLimit =
      ServiceConfig::builder().withMaxConnections(10).build();
  EXPECT_EQ(lowLimit.getMaxConnections(), 10);

  auto highLimit =
      ServiceConfig::builder().withMaxConnections(10000).build();
  EXPECT_EQ(highLimit.getMaxConnections(), 10000);
}

TEST(ServiceConfigTest, TimeoutValues) {
  auto shortTimeout =
      ServiceConfig::builder().withRequestTimeoutMs(100).build();
  EXPECT_EQ(shortTimeout.getRequestTimeoutMs(), 100);

  auto longTimeout =
      ServiceConfig::builder().withRequestTimeoutMs(60000).build();
  EXPECT_EQ(longTimeout.getRequestTimeoutMs(), 60000);
}

TEST(ServiceConfigTest, DifferentHosts) {
  auto localhost =
      ServiceConfig::builder().withHost("127.0.0.1").build();
  EXPECT_EQ(localhost.getHost(), "127.0.0.1");

  auto allInterfaces =
      ServiceConfig::builder().withHost("0.0.0.0").build();
  EXPECT_EQ(allInterfaces.getHost(), "0.0.0.0");

  auto ipv6 = ServiceConfig::builder().withHost("::1").build();
  EXPECT_EQ(ipv6.getHost(), "::1");
}

TEST(ServiceConfigTest, BuilderIndependence) {
  auto builder1 = ServiceConfig::builder().withPort(9000);
  auto builder2 = ServiceConfig::builder().withPort(9001);

  auto config1 = builder1.build();
  auto config2 = builder2.build();

  EXPECT_EQ(config1.getPort(), 9000);
  EXPECT_EQ(config2.getPort(), 9001);
}

TEST(ServiceConfigTest, ComplexConfiguration) {
  auto config = ServiceConfig::builder()
                    .withServiceName("ComplexService")
                    .withHost("192.168.1.1")
                    .withPort(8443)
                    .withMaxConnections(1000)
                    .withRequestTimeoutMs(30000)
                    .withEnableSSL(true)
                    .withCertPath("/etc/certs/server.crt")
                    .withKeyPath("/etc/certs/server.key")
                    .build();

  EXPECT_EQ(config.getServiceName(), "ComplexService");
  EXPECT_EQ(config.getHost(), "192.168.1.1");
  EXPECT_EQ(config.getPort(), 8443);
  EXPECT_EQ(config.getMaxConnections(), 1000);
  EXPECT_EQ(config.getRequestTimeoutMs(), 30000);
  EXPECT_TRUE(config.isSSLEnabled());
  EXPECT_EQ(config.getCertPath(), "/etc/certs/server.crt");
  EXPECT_EQ(config.getKeyPath(), "/etc/certs/server.key");
}
