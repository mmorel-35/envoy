// NOLINT(namespace-envoy)
#include <cstdlib>
#include <iostream>

#include "google/protobuf/descriptor.h"

// Include service headers to force descriptor registration
#include "envoy/api/v2/cds.pb.h"
#include "envoy/api/v2/eds.pb.h"
#include "envoy/api/v2/lds.pb.h"
#include "envoy/api/v2/rds.pb.h"
#include "envoy/service/accesslog/v2/als.pb.h"
#include "envoy/service/discovery/v2/ads.pb.h"
#include "envoy/service/discovery/v2/hds.pb.h"
#include "envoy/service/discovery/v2/rtds.pb.h"
#include "envoy/service/metrics/v2/metrics_service.pb.h"
#include "envoy/service/ratelimit/v2/rls.pb.h"

// Basic C++ build/link validation for the v2 xDS APIs.
int main(int argc, char* argv[]) {
  // Force service descriptor registration by calling descriptor() methods
  (void)envoy::api::v2::ClusterDiscoveryService::descriptor();
  (void)envoy::api::v2::EndpointDiscoveryService::descriptor();
  (void)envoy::api::v2::ListenerDiscoveryService::descriptor();
  (void)envoy::api::v2::RouteDiscoveryService::descriptor();
  (void)envoy::service::discovery::v2::AggregatedDiscoveryService::descriptor();
  (void)envoy::service::discovery::v2::HealthDiscoveryService::descriptor();
  (void)envoy::service::discovery::v2::RuntimeDiscoveryService::descriptor();
  (void)envoy::service::accesslog::v2::AccessLogService::descriptor();
  (void)envoy::service::metrics::v2::MetricsService::descriptor();
  (void)envoy::service::ratelimit::v2::RateLimitService::descriptor();
  // Note: udpa.service.orca.v1.OpenRcaService does not have cc_generic_services enabled,
  // so we cannot check it here.

  const auto methods = {
      "envoy.api.v2.ClusterDiscoveryService.FetchClusters",
      "envoy.api.v2.ClusterDiscoveryService.StreamClusters",
      "envoy.api.v2.EndpointDiscoveryService.FetchEndpoints",
      "envoy.api.v2.EndpointDiscoveryService.StreamEndpoints",
      "envoy.api.v2.ListenerDiscoveryService.FetchListeners",
      "envoy.api.v2.ListenerDiscoveryService.StreamListeners",
      "envoy.api.v2.RouteDiscoveryService.FetchRoutes",
      "envoy.api.v2.RouteDiscoveryService.StreamRoutes",
      "envoy.service.discovery.v2.AggregatedDiscoveryService.StreamAggregatedResources",
      "envoy.service.discovery.v2.HealthDiscoveryService.FetchHealthCheck",
      "envoy.service.discovery.v2.HealthDiscoveryService.StreamHealthCheck",
      "envoy.service.discovery.v2.RuntimeDiscoveryService.FetchRuntime",
      "envoy.service.discovery.v2.RuntimeDiscoveryService.StreamRuntime",
      "envoy.service.accesslog.v2.AccessLogService.StreamAccessLogs",
      "envoy.service.metrics.v2.MetricsService.StreamMetrics",
      "envoy.service.ratelimit.v2.RateLimitService.ShouldRateLimit",
      // Note: Skipping udpa.service.orca.v1.OpenRcaService.StreamCoreMetrics
      // as the external xds repository does not have cc_generic_services enabled.
  };

  for (const auto& method : methods) {
    if (google::protobuf::DescriptorPool::generated_pool()->FindMethodByName(method) == nullptr) {
      std::cout << "Unable to find method descriptor for " << method << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  exit(EXIT_SUCCESS);
}
