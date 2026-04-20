#include "config/ServiceConfig.hpp"

namespace trading::config {

ServiceConfig::Builder& ServiceConfig::Builder::withServiceName(
    const std::string& name) noexcept {
  serviceName = name;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withPort(
    std::int32_t port) noexcept {
  ServiceConfig::Builder::port = port;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withHost(
    const std::string& host) noexcept {
  ServiceConfig::Builder::host = host;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withMaxConnections(
    std::int32_t maxConns) noexcept {
  maxConnections = maxConns;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withRequestTimeoutMs(
    std::int32_t timeoutMs) noexcept {
  requestTimeoutMs = timeoutMs;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withEnableSSL(
    bool enabled) noexcept {
  enableSSL = enabled;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withCertPath(
    const std::string& path) noexcept {
  certPath = path;
  return *this;
}

ServiceConfig::Builder& ServiceConfig::Builder::withKeyPath(
    const std::string& path) noexcept {
  keyPath = path;
  return *this;
}

ServiceConfig ServiceConfig::Builder::build() const {
  return ServiceConfig(*this);
}

ServiceConfig::ServiceConfig(const Builder& builder)
    : serviceName_(builder.serviceName),
      port_(builder.port),
      host_(builder.host),
      maxConnections_(builder.maxConnections),
      requestTimeoutMs_(builder.requestTimeoutMs),
      enableSSL_(builder.enableSSL),
      certPath_(builder.certPath),
      keyPath_(builder.keyPath) {}

const std::string& ServiceConfig::getServiceName() const noexcept {
  return serviceName_;
}

std::int32_t ServiceConfig::getPort() const noexcept { return port_; }

const std::string& ServiceConfig::getHost() const noexcept { return host_; }

std::int32_t ServiceConfig::getMaxConnections() const noexcept {
  return maxConnections_;
}

std::int32_t ServiceConfig::getRequestTimeoutMs() const noexcept {
  return requestTimeoutMs_;
}

bool ServiceConfig::isSSLEnabled() const noexcept { return enableSSL_; }

const std::string& ServiceConfig::getCertPath() const noexcept {
  return certPath_;
}

const std::string& ServiceConfig::getKeyPath() const noexcept {
  return keyPath_;
}

ServiceConfig::Builder ServiceConfig::builder() noexcept { return Builder{}; }

}  // namespace trading::config
