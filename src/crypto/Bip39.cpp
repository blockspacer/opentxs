// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "1_Internal.hpp"    // IWYU pragma: associated
#include "crypto/Bip39.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "2_Factory.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/protobuf/Enums.pb.h"

#define OT_METHOD "opentxs::crypto::implementation::Bip39::"

namespace opentxs
{
auto Factory::Bip39(const api::Crypto& api) noexcept
    -> std::unique_ptr<crypto::Bip39>
{
    using ReturnType = crypto::implementation::Bip39;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
const std::size_t Bip39::BitsPerWord{11};
const std::uint8_t Bip39::ByteBits{8};
const std::size_t Bip39::DictionarySize{2048};
const std::size_t Bip39::EntropyBitDivisor{32};
const std::size_t Bip39::HmacOutputSizeBytes{64};
const std::size_t Bip39::HmacIterationCount{2048};
const std::string Bip39::PassphrasePrefix{"mnemonic"};
const std::size_t Bip39::ValidMnemonicWordMultiple{3};

Bip39::Bip39(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
{
}

auto Bip39::bitShift(std::size_t theBit) noexcept -> std::byte
{
    return static_cast<std::byte>(1 << (ByteBits - (theBit % ByteBits) - 1));
}

auto Bip39::entropy_to_words(const Secret& entropy, Secret& words)
    const noexcept -> bool
{
    const auto bytes = entropy.Bytes();

    switch (bytes.size()) {
        case 16:
        case 20:
        case 24:
        case 28:
        case 32:
            break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid entropy size: ")(
                bytes.size())
                .Flush();

            return false;
        }
    }

    const auto entropyBitCount = std::size_t{bytes.size() * ByteBits};
    const auto checksumBits = std::size_t{entropyBitCount / EntropyBitDivisor};
    const auto entropyPlusCheckBits =
        std::size_t{entropyBitCount + checksumBits};
    const auto wordCount = std::size_t{entropyPlusCheckBits / BitsPerWord};

    if (0 != (wordCount % ValidMnemonicWordMultiple)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": (0 != (wordCount % ValidMnemonicWordMultiple))")
            .Flush();

        return false;
    }

    if (0 != (entropyPlusCheckBits % BitsPerWord)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": (0 != (entropyPlusCheckBits % BitsPerWord))")
            .Flush();

        return false;
    }

    auto newData = opentxs::Data::Factory();  // TODO should be secret
    auto digestOutput = opentxs::Data::Factory();

    if (false == crypto_.Hash().Digest(
                     opentxs::proto::HashType::HASHTYPE_SHA256,
                     bytes,
                     digestOutput->WriteInto())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Digest(opentxs::proto::HashType::HASHTYPE_SHA256...) failed.")
            .Flush();

        return false;
    } else {
        newData->Concatenate(bytes);
        newData += digestOutput;
    }

    auto mnemonicWords = MnemonicWords{};
    auto bitIndex = std::size_t{0};

    for (std::size_t currentWord = 0; currentWord < wordCount; currentWord++) {
        auto indexDict = std::size_t{0};

        for (std::size_t bit_iterator = 0; bit_iterator < BitsPerWord;
             bit_iterator++) {
            bitIndex = ((BitsPerWord * currentWord) + bit_iterator);
            indexDict <<= 1;
            const auto byteIndex =
                bitIndex / static_cast<std::size_t>(ByteBits);
            auto indexed_byte = std::byte{0};
            const bool bExtracted = newData->Extract(
                reinterpret_cast<std::uint8_t&>(indexed_byte), byteIndex);

            if (!bExtracted) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": (!bExtracted) -- returning")
                    .Flush();

                return false;
            }

            if (std::to_integer<std::uint8_t>(
                    indexed_byte & bitShift(bitIndex)) > 0) {
                indexDict++;
            }
        }

        OT_ASSERT(indexDict < DictionarySize);

        const auto& dictionary = words_.at("en");
        const auto& theString = dictionary.at(indexDict);
        mnemonicWords.push_back(theString);
    }

    if (mnemonicWords.size() != ((bitIndex + 1) / BitsPerWord)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": (mnemonicWords.size() != ((bitIndex + 1) / BitsPerWord))")
            .Flush();

        return false;
    }

    auto output = std::string{};
    auto nIndex = int{-1};

    for (const auto& word : mnemonicWords) {
        ++nIndex;

        if (nIndex > 0) { output += " "; }

        output += word;
    }

    words.AssignText(output);

    return true;
}

auto Bip39::SeedToWords(const Secret& seed, Secret& words) const noexcept
    -> bool
{
    return entropy_to_words(seed, words);
}

auto Bip39::words_to_root(
    const Secret& words,
    Secret& bip32RootNode,
    const Secret& passphrase) const noexcept -> void
{
    auto salt = std::string{PassphrasePrefix};

    if (passphrase.size() > 0) { salt += std::string{passphrase.Bytes()}; }

    auto dataOutput = opentxs::Data::Factory();  // TODO should be secret
    const auto dataSalt = opentxs::Data::Factory(salt.data(), salt.size());
    crypto_.Hash().PKCS5_PBKDF2_HMAC(
        words,
        dataSalt,
        HmacIterationCount,
        proto::HashType::HASHTYPE_SHA512,
        HmacOutputSizeBytes,
        dataOutput);
    bip32RootNode.Assign(dataOutput->Bytes());
}

auto Bip39::WordsToSeed(
    const Secret& words,
    Secret& seed,
    const Secret& passphrase) const noexcept -> void
{
    words_to_root(words, seed, passphrase);
}
}  // namespace opentxs::crypto::implementation
