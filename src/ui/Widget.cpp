// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/Widget.hpp"   // IWYU pragma: associated

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::ui::implementation
{
Widget::Widget(
    const api::client::internal::Manager& api,
    const Identifier& id,
    const SimpleCallback& cb) noexcept
    : api_(api)
    , widget_id_(id)
    , callbacks_()
    , listeners_()
{
    if (cb) { SetCallback(cb); }
}

void Widget::setup_listeners(const ListenerDefinitions& definitions) noexcept
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const network::zeromq::Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket = listeners_.emplace_back(
            api_.ZeroMQ().SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}
}  // namespace opentxs::ui::implementation
