// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/message/Getblocktxn.cpp"

#pragma once

#include <iosfwd>
#include <memory>
#include <new>
#include <set>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Getblocktxn final : virtual public bitcoin::Message
{
public:
    auto getBlockHash() const noexcept -> OTData
    {
        return Data::Factory(block_hash_);
    }
    auto getIndices() const noexcept -> const std::vector<std::size_t>&
    {
        return txn_indices_;
    }

    Getblocktxn(
        const api::client::Manager& api,
        const blockchain::Type network,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept;
    Getblocktxn(
        const api::client::Manager& api,
        std::unique_ptr<Header> header,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept(false);

    ~Getblocktxn() final = default;

private:
    const OTData block_hash_;
    const std::vector<std::size_t> txn_indices_;

    auto payload() const noexcept -> OTData final;

    Getblocktxn(const Getblocktxn&) = delete;
    Getblocktxn(Getblocktxn&&) = delete;
    auto operator=(const Getblocktxn&) -> Getblocktxn& = delete;
    auto operator=(Getblocktxn &&) -> Getblocktxn& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
