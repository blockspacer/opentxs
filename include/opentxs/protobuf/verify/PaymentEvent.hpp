// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_PAYMENTEVENT_HPP
#define OPENTXS_PROTOBUF_PAYMENTEVENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"

namespace opentxs
{
namespace proto
{
class PaymentEvent;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT bool CheckProto_1(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_2(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_3(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_4(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_5(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_6(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_7(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_8(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_9(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_10(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_11(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_12(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_13(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_14(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_15(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_16(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_17(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_18(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_19(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
OPENTXS_EXPORT bool CheckProto_20(
    const PaymentEvent& input,
    const bool silent,
    const std::uint32_t parentVersion,
    const PaymentWorkflowType parent,
    std::map<PaymentEventType, std::size_t>& events);
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_PAYMENTEVENT_HPP
