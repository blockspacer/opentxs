// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <secp256k1.h>
}
#include <iosfwd>
#include <optional>

#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/Secp256k1.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Util;
}  // namespace crypto

namespace internal
{
struct Core;
}  // namespace internal

class Crypto;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

class Factory;
class NymParameters;
class OTPassword;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
class Secp256k1 final : virtual public crypto::Secp256k1,
                        public AsymmetricProvider,
                        public EcdsaProvider
{
public:
    auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const proto::KeyRole role,
        const NymParameters& options,
        const AllocateOutput params) const noexcept -> bool final;
    auto ScalarAdd(
        const ReadView lhs,
        const ReadView rhs,
        const AllocateOutput result) const noexcept -> bool final;
    auto ScalarMultiplyBase(const ReadView scalar, const AllocateOutput result)
        const noexcept -> bool final;
    auto SharedSecret(
        const key::Asymmetric& publicKey,
        const key::Asymmetric& privateKey,
        const PasswordPrompt& reason,
        Secret& secret) const noexcept -> bool final;
    auto Sign(
        const api::internal::Core& api,
        const ReadView plaintext,
        const key::Asymmetric& key,
        const proto::HashType hash,
        const AllocateOutput signature,
        const PasswordPrompt& reason) const -> bool final;
    auto SignDER(
        const api::internal::Core& api,
        const ReadView plaintext,
        const key::Asymmetric& key,
        const proto::HashType hash,
        Space& signature,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType) const -> bool final;

    void Init() final;

    ~Secp256k1() final;

private:
    friend opentxs::Factory;

    static const std::size_t PrivateKeySize{32};
    static const std::size_t PublicKeySize{33};
    static bool Initialized_;

    secp256k1_context* context_;
    const api::crypto::Util& ssl_;

    auto hash(const proto::HashType type, const ReadView data) const
        noexcept(false) -> OTData;
    auto parsed_public_key(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_pubkey;
    auto parsed_signature(const ReadView bytes) const noexcept(false)
        -> ::secp256k1_ecdsa_signature;

    Secp256k1(const api::Crypto& crypto, const api::crypto::Util& ssl);
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1 &&) -> Secp256k1& = delete;
};
}  // namespace opentxs::crypto::implementation
