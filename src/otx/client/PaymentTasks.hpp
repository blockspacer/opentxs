// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Identifier.hpp"

#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/otx/client/Client.hpp"

#include "DepositPayment.hpp"

#include <map>

namespace opentxs::otx::client::implementation
{
class PaymentTasks final : public opentxs::internal::StateMachine
{
public:
    using BackgroundTask = api::client::OTX::BackgroundTask;

    BackgroundTask Queue(const DepositPaymentTask& task);

    PaymentTasks(client::internal::StateMachine& parent);
    ~PaymentTasks() = default;

private:
    using Future = api::client::OTX::Future;
    using TaskMap = std::map<OTIdentifier, DepositPayment>;

    static BackgroundTask error_task();

    client::internal::StateMachine& parent_;
    TaskMap tasks_;

    bool cleanup();
    OTIdentifier get_payment_id(const OTPayment& payment) const;

    PaymentTasks() = delete;
};
}  // namespace opentxs::otx::client::implementation
