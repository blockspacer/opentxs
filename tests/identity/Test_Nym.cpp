// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

namespace ot = opentxs;

namespace
{
class Test_Nym : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    const ot::OTPasswordPrompt reason_;

    Test_Nym()
        : client_(ot::Context().StartClient({}, 0))
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
    {
    }
};

TEST_F(Test_Nym, init_ot) {}

TEST_F(Test_Nym, default_params)
{
    auto params = ot::NymParameters{};
    const auto nym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(nym);
}

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, secp256k1_hd_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, secp256k1_hd_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, secp256k1_legacy_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    EXPECT_FALSE(pNym);
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, secp256k1_legacy_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::secp256k1);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, ed25519_hd_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, ed25519_hd_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_HD);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_Nym, ed25519_legacy_bip47)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_BIP47);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    EXPECT_FALSE(pNym);
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_Nym, ed25519_legacy_self_signed)
{
    auto params = ot::NymParameters{};
    params.setNymParameterType(ot::NymParameterType::ed25519);
    params.setCredentialType(ot::proto::CREDTYPE_LEGACY);
    params.SetSourceType(ot::proto::SOURCETYPE_PUBKEY);

    const auto pNym = client_.Wallet().Nym(params, reason_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;

    EXPECT_TRUE(nym.Alias().empty());
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::ENCRYPT_MESSAGE));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::AUTHENTICATE_CONNECTION));
    EXPECT_TRUE(nym.HasCapability(ot::NymCapability::SIGN_CHILDCRED));
    EXPECT_EQ(1, nym.Revision());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace