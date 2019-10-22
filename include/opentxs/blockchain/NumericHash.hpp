// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NUMERIC_HASH_HPP
#define OPENTXS_BLOCKCHAIN_NUMERIC_HASH_HPP

#include "opentxs/Forward.hpp"

#include <string>

namespace opentxs
{
bool operator==(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
bool operator!=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
bool operator<(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;
bool operator<=(
    const OTNumericHash& lhs,
    const blockchain::NumericHash& rhs) noexcept;

namespace blockchain
{
class NumericHash
{
public:
    EXPORT static const std::int32_t MaxTarget;

    EXPORT virtual bool operator==(const blockchain::NumericHash& rhs) const
        noexcept = 0;
    EXPORT virtual bool operator!=(const blockchain::NumericHash& rhs) const
        noexcept = 0;
    EXPORT virtual bool operator<(const blockchain::NumericHash& rhs) const
        noexcept = 0;
    EXPORT virtual bool operator<=(const blockchain::NumericHash& rhs) const
        noexcept = 0;

    EXPORT virtual std::string asHex(const std::size_t minimumBytes = 32) const
        noexcept = 0;
    EXPORT virtual std::string Decimal() const noexcept = 0;

    EXPORT virtual ~NumericHash() = default;

protected:
    NumericHash() noexcept = default;

private:
    friend OTNumericHash;

    virtual NumericHash* clone() const noexcept = 0;

    NumericHash(const NumericHash& rhs) = delete;
    NumericHash(NumericHash&& rhs) = delete;
    NumericHash& operator=(const NumericHash& rhs) = delete;
    NumericHash& operator=(NumericHash&& rhs) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif