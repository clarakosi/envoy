#include "extensions/filters/http/ratelimit/ratelimit_policy_impl.h"

#include "common/config/metadata.h"
#include "common/http/header_utility.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace RateLimitFilter {

const uint64_t FilterRateLimitPolicyImpl::MAX_STAGE_NUMBER = 10UL;

FilterRateLimitPolicyEntryImpl::FilterRateLimitPolicyEntryImpl(
    const envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit& config)
    : Router::RateLimitPolicyEntryImpl(
          dynamic_cast<const envoy::config::route::v3::RateLimit&>(config)) {
  for (const auto& action : config.actions()) {
    switch (action.action_specifier_case()) {
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kSourceCluster:
      actions_.emplace_back(new Router::SourceClusterAction());
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kDestinationCluster:
      actions_.emplace_back(new Router::DestinationClusterAction());
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kRequestHeaders:
      actions_.emplace_back(new FilterRequestHeadersAction(action.request_headers()));
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kRemoteAddress:
      actions_.emplace_back(new Router::RemoteAddressAction());
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kGenericKey:
      actions_.emplace_back(new FilterGenericKeyAction(action.generic_key()));
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kDynamicMetadata:
      actions_.emplace_back(new FilterDynamicMetaDataAction(action.dynamic_metadata()));
      break;
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit::Action::
        ActionSpecifierCase::kHeaderValueMatch:
      actions_.emplace_back(new FilterHeaderValueMatchAction(action.header_value_match()));
      break;
    default:
      NOT_REACHED_GCOVR_EXCL_LINE;
    }
  }
  if (config.has_limit()) {
    switch (config.limit().override_specifier_case()) {
    case envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute_RateLimit_Override::
        OverrideSpecifierCase::kDynamicMetadata:
      limit_override_.emplace(
          new FilterDynamicMetadataRateLimitOverride(config.limit().dynamic_metadata()));
      break;
    default:
      NOT_REACHED_GCOVR_EXCL_LINE;
    }
  }
}

FilterRateLimitPolicyImpl::FilterRateLimitPolicyImpl(
    const Protobuf::RepeatedPtrField<
        envoy::extensions::filters::http::ratelimit::v3::RateLimitPerRoute::RateLimit>& rate_limits)
    : rate_limit_entries_reference_(FilterRateLimitPolicyImpl::MAX_STAGE_NUMBER + 1) {
  for (const auto& rate_limit : rate_limits) {
    std::unique_ptr<Router::RateLimitPolicyEntry> rate_limit_policy_entry(
        new FilterRateLimitPolicyEntryImpl(rate_limit));
    uint64_t stage = rate_limit_policy_entry->stage();
    ASSERT(stage < rate_limit_entries_reference_.size());
    rate_limit_entries_reference_[stage].emplace_back(*rate_limit_policy_entry);
    rate_limit_entries_.emplace_back(std::move(rate_limit_policy_entry));
  }
}

const std::vector<std::reference_wrapper<const Router::RateLimitPolicyEntry>>&
FilterRateLimitPolicyImpl::getApplicableRateLimit(uint64_t stage) const {
  ASSERT(stage < rate_limit_entries_reference_.size());
  return rate_limit_entries_reference_[stage];
}

} // namespace RateLimitFilter
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy