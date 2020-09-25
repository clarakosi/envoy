#pragma once

#include "envoy/config/core/v3/base.pb.h"
#include "envoy/extensions/filters/http/ratelimit/v3/rate_limit.pb.h"
#include "envoy/router/router.h"
#include "envoy/router/router_ratelimit.h"

#include "common/config/metadata.h"
#include "common/http/header_utility.h"
#include "common/router/router_ratelimit.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace RateLimitFilter {

/**
 * Populate rate limit override from dynamic metadata.
 */
class FilterDynamicMetadataRateLimitOverride : public Router::DynamicMetadataRateLimitOverride {
public:
  explicit FilterDynamicMetadataRateLimitOverride(
      const envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::
          Override::DynamicMetadata& config)
      : Router::DynamicMetadataRateLimitOverride(
            dynamic_cast<const envoy::config::route::v3::RateLimit::Override::DynamicMetadata&>(
                config)){};
};

/**
 * Action for request headers rate limiting.
 */
class FilterRequestHeadersAction : public Router::RequestHeadersAction {
public:
  FilterRequestHeadersAction(const envoy::extensions::filters::http::ratelimit::v3::
                                 RateLimitPerRoute::RateLimit::Action::RequestHeaders& action)
      : Router::RequestHeadersAction(
            dynamic_cast<const envoy::config::route::v3::RateLimit::Action::RequestHeaders&>(
                action)) {}
};

/**
 * Action for generic key rate limiting.
 */
class FilterGenericKeyAction : public Router::GenericKeyAction {
public:
  FilterGenericKeyAction(const envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::
                             RateLimit::Action::GenericKey& action)
      : Router::GenericKeyAction(
            dynamic_cast<const envoy::config::route::v3::RateLimit::Action::GenericKey&>(action)) {}
};

/**
 * Action for dynamic metadata rate limiting.
 */
class FilterDynamicMetaDataAction : public Router::DynamicMetaDataAction {
public:
  FilterDynamicMetaDataAction(const envoy::extensions::filters::http::ratelimit::v3::
                                  RateLimitPerRoute::RateLimit::Action::DynamicMetaData& action)
      : Router::DynamicMetaDataAction(
            dynamic_cast<const envoy::config::route::v3::RateLimit::Action::DynamicMetaData&>(
                action)) {}
};

/**
 * Action for header value match rate limiting.
 */
class FilterHeaderValueMatchAction : public Router::HeaderValueMatchAction {
public:
  FilterHeaderValueMatchAction(const envoy::extensions::filters::http::ratelimit::v3::
                                   RateLimitPerRoute::RateLimit::Action::HeaderValueMatch& action)
      : Router::HeaderValueMatchAction(
            dynamic_cast<const envoy::config::route::v3::RateLimit::Action::HeaderValueMatch&>(
                action)) {}
};

/**
 * Implementation of RateLimitPolicyEntry that holds the action for the configuration.
 */
class FilterRateLimitPolicyEntryImpl : public Router::RateLimitPolicyEntryImpl {
public:
  FilterRateLimitPolicyEntryImpl(
      const envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit& config);
};

/**
 * Implementation of RateLimitPolicy that reads from the JSON route config.
 */
class FilterRateLimitPolicyImpl : public Router::RateLimitPolicy {
public:
  FilterRateLimitPolicyImpl(
      const Protobuf::RepeatedPtrField<
          envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit>&
          rate_limits);

  // Router::RateLimitPolicy
  const std::vector<std::reference_wrapper<const Router::RateLimitPolicyEntry>>&
  getApplicableRateLimit(uint64_t stage = 0) const override;
  bool empty() const override { return rate_limit_entries_.empty(); }

private:
  std::vector<std::unique_ptr<Router::RateLimitPolicyEntry>> rate_limit_entries_;
  std::vector<std::vector<std::reference_wrapper<const Router::RateLimitPolicyEntry>>>
      rate_limit_entries_reference_;
  // The maximum stage number supported. This value should match the maximum stage number in
  // Json::Schema::HTTP_RATE_LIMITS_CONFIGURATION_SCHEMA and
  // Json::Schema::RATE_LIMIT_HTTP_FILTER_SCHEMA from common/json/config_schemas.cc.
  static const uint64_t MAX_STAGE_NUMBER;
};

} // namespace RateLimitFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy