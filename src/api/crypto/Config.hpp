// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/crypto/Config.cpp"

#pragma once

#include <cstdint>
#include <string>

#include "opentxs/api/crypto/Config.hpp"

namespace opentxs
{
namespace api
{
class Settings;
}  // namespace api

class Factory;
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
class Config final : public api::crypto::Config
{
public:
    auto IterationCount() const -> std::uint32_t override;
    auto SymmetricSaltSize() const -> std::uint32_t override;
    auto SymmetricKeySize() const -> std::uint32_t override;
    auto SymmetricKeySizeMax() const -> std::uint32_t override;
    auto SymmetricIvSize() const -> std::uint32_t override;
    auto SymmetricBufferSize() const -> std::uint32_t override;
    auto PublicKeysize() const -> std::uint32_t override;
    auto PublicKeysizeMax() const -> std::uint32_t override;

private:
    friend opentxs::Factory;

    const api::Settings& config_;
    mutable std::int32_t sp_nIterationCount{0};
    mutable std::int32_t sp_nSymmetricSaltSize{0};
    mutable std::int32_t sp_nSymmetricKeySize{0};
    mutable std::int32_t sp_nSymmetricKeySizeMax{0};
    mutable std::int32_t sp_nSymmetricIvSize{0};
    mutable std::int32_t sp_nSymmetricBufferSize{0};
    mutable std::int32_t sp_nPublicKeysize{0};
    mutable std::int32_t sp_nPublicKeysizeMax{0};

    auto GetSetAll() const -> bool;
    auto GetSetValue(
        const std::string& strKeyName,
        const std::int32_t nDefaultValue,
        std::int32_t& out_nValue) const -> bool;

    Config(const api::Settings& settings);
    Config() = delete;
    Config(const Config&) = delete;
    Config(Config&&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    auto operator=(Config &&) -> Config& = delete;
};
}  // namespace opentxs::api::crypto::implementation
