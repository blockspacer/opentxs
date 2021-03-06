// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "1_Internal.hpp"     // IWYU pragma: associated
#include "crypto/HDNode.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <iterator>

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/HD.hpp"

namespace opentxs::crypto::implementation
{
HDNode::HDNode(const api::Crypto& crypto) noexcept
    : data_space_(Context().Factory().Secret(0))
    , hash_space_(Context().Factory().Secret(0))
    , data_(data_space_->WriteInto()(33 + 4))
    , hash_(hash_space_->WriteInto()(64))
    , crypto_(crypto)
    , switch_(0)
    , a_(Context().Factory().Secret(0))
    , b_(Context().Factory().Secret(0))
{
    OT_ASSERT(data_.valid(33 + 4));
    OT_ASSERT(hash_.valid(64));

    {
        static const auto size = std::size_t{32 + 32 + 33};
        a_->WriteInto()(size);
        b_->WriteInto()(size);

        OT_ASSERT(size == a_->size());
        OT_ASSERT(size == b_->size());
    }
}

auto HDNode::child() noexcept -> Secret&
{
    return (1 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::ChildCode() noexcept -> WritableView
{
    auto start = child().data();
    std::advance(start, 32);

    return WritableView{start, 32};
}

auto HDNode::ChildPrivate() noexcept -> AllocateOutput
{
    auto start = child().data();

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::ChildPublic() noexcept -> AllocateOutput
{
    auto start = child().data();
    std::advance(start, 32 + 32);

    return [start](const auto) { return WritableView{start, 33}; };
}

auto HDNode::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return key::HD::CalculateFingerprint(crypto_.Hash(), ParentPublic());
}

auto HDNode::InitCode() noexcept -> AllocateOutput
{
    auto start = parent().data();
    std::advance(start, 32);

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::InitPrivate() noexcept -> AllocateOutput
{
    auto start = parent().data();

    return [start](const auto) { return WritableView{start, 32}; };
}

auto HDNode::InitPublic() noexcept -> AllocateOutput
{
    auto start = parent().data();
    std::advance(start, 32 + 32);

    return [start](const auto) { return WritableView{start, 33}; };
}

auto HDNode::Next() noexcept -> void { ++switch_; }

auto HDNode::parent() const noexcept -> const Secret&
{
    return (0 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::parent() noexcept -> Secret&
{
    return (0 == (switch_ % 2)) ? a_ : b_;
}

auto HDNode::ParentCode() const noexcept -> ReadView
{
    auto start{parent().Bytes().data()};
    std::advance(start, 32);

    return ReadView{start, 32};
}

auto HDNode::ParentPrivate() const noexcept -> ReadView
{
    auto start{parent().Bytes().data()};

    return ReadView{start, 32};
}

auto HDNode::ParentPublic() const noexcept -> ReadView
{
    auto start{parent().Bytes().data()};
    std::advance(start, 32 + 32);

    return ReadView{start, 33};
}
}  // namespace opentxs::crypto::implementation
