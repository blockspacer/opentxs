/// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"

#include "ZAP.hpp"

//#define OT_METHOD "opentxs::api::network::ZAP::"

namespace opentxs
{
api::network::ZAP* Factory::ZAP(const network::zeromq::Context& context)
{
    return new api::network::implementation::ZAP(context);
}
}  // namespace opentxs

namespace opentxs::api::network::implementation
{
ZAP::ZAP(const opentxs::network::zeromq::Context& context)
    : context_(context)
    , callback_(opentxs::network::zeromq::zap::Callback::Factory())
    , zap_(opentxs::network::zeromq::zap::Handler::Factory(context_, callback_))
{
}

bool ZAP::RegisterDomain(const std::string& domain, const Callback& callback)
    const
{
    return callback_->SetDomain(domain, callback);
}

bool ZAP::SetDefaultPolicy(const Policy policy) const
{
    return callback_->SetPolicy(policy);
}
}  // namespace opentxs::api::network::implementation