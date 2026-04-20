#pragma once

#include <cstdint>
#include <string>

namespace trading::config {

class ServiceConfig {
 public:
  struct Builder {
    Builder& withServiceName(const std::string& name) noexcept;
    Builder& withPort(std::int32_t port) noexcept;
    Builder& withHost(const std::string& host) noexcept;
    Builder& withMaxConnections(std::int32_t maxConns) noexcept;
    Builder& withRequestTimeoutMs(std::int32_t timeoutMs) noexcept;
    Builder& withEnableSSL(bool enabled) noexcept;
    Builder& withCertPath(const std::string& path) noexcept;
    Builder& withKeyPath(const std::string& path) noexcept;

    ServiceConfig build() const;

    std::string serviceName{"MatchingService"};
    std::int32_t port{50051};
    std::string host{"0.0.0.0"};
    std::int32_t maxConnections{100};
    std::int32_t requestTimeoutMs{5000};
    bool enableSSL{false};
    std::string certPath{};
    std::string keyPath{};
  };

  ServiceConfig(const Builder& builder);

  const std::string& getServiceName() const noexcept;
  std::int32_t getPort() const noexcept;
  const std::string& getHost() const noexcept;
  std::int32_t getMaxConnections() const noexcept;
  std::int32_t getRequestTimeoutMs() const noexcept;
  bool isSSLEnabled() const noexcept;
  const std::string& getCertPath() const noexcept;
  const std::string& getKeyPath() const noexcept;

  static Builder builder() noexcept;

 private:
  std::string serviceName_;
  std::int32_t port_;
  std::string host_;
  std::int32_t maxConnections_;
  std::int32_t requestTimeoutMs_;
  bool enableSSL_;
  std::string certPath_;
  std::string keyPath_;
};

}  // namespace trading::config
