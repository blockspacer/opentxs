// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database

namespace internal
{
struct BalanceTree;
}  // namespace internal
}  // namespace blockchain

class Issuer;
class ServerAction;
class Workflow;
}  // namespace client

class Crypto;
class Legacy;
class Settings;
class Wallet;
}  // namespace api

namespace blockchain
{
namespace client
{
namespace internal
{
struct IO;
struct ThreadPool;
}  // namespace internal
}  // namespace client
}  // namespace blockchain

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Issuer;
}  // namespace proto

class Flag;
class Identifier;
class OTClient;
class OTPayment;
template <class T>
class UniqueQueue;
}  // namespace opentxs

namespace opentxs
{
auto Translate(const blockchain::Type type) noexcept -> proto::ContactItemType;
auto Translate(const proto::ContactItemType type) noexcept -> blockchain::Type;
}  // namespace opentxs

namespace opentxs::api::client::internal
{
struct Activity : virtual public api::client::Activity {
    virtual void MigrateLegacyThreads() const = 0;

    virtual ~Activity() = default;
};
struct Blockchain : virtual public api::client::Blockchain {
    virtual auto API() const noexcept -> const api::internal::Core& = 0;
    /// Throws std::runtime_error if type is invalid
    OPENTXS_EXPORT virtual auto BalanceTree(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false)
        -> const blockchain::internal::BalanceTree& = 0;
#if OT_BLOCKCHAIN
    virtual auto BlockchainDB() const noexcept
        -> const blockchain::database::implementation::Database& = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const blockchain::AddressStyle format,
        const Data& pubkey) const noexcept -> std::string = 0;
    virtual auto Contacts() const noexcept -> const api::client::Contacts& = 0;
#if OT_BLOCKCHAIN
    virtual auto IO() const noexcept
        -> const opentxs::blockchain::client::internal::IO& = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto PubkeyHash(
        const opentxs::blockchain::Type chain,
        const Data& pubkey) const noexcept(false) -> OTData = 0;
#if OT_BLOCKCHAIN
    virtual auto Reorg() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto ThreadPool() const noexcept
        -> const opentxs::blockchain::client::internal::ThreadPool& = 0;
    virtual auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void = 0;
#endif  // OT_BLOCKCHAIN
    virtual auto UpdateElement(
        std::vector<ReadView>& pubkeyHashes) const noexcept -> void = 0;

    virtual ~Blockchain() = default;
};
struct Contacts : virtual public api::client::Contacts {
    virtual void start() = 0;

    virtual ~Contacts() = default;
};
struct Manager : virtual public api::client::Manager,
                 virtual public api::internal::Core {
    virtual auto ActivateUICallback(const Identifier& widget) const noexcept
        -> void = 0;
    virtual auto RegisterUICallback(
        const Identifier& widget,
        const SimpleCallback& cb) const noexcept -> void = 0;
    virtual void StartActivity() = 0;
    virtual void StartContacts() = 0;

    virtual ~Manager() = default;
};

struct OTX : virtual public api::client::OTX {
    virtual void associate_message_id(
        const Identifier& messageID,
        const TaskID taskID) const = 0;
    virtual auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const -> Depositability = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    virtual auto get_nym_fetch(const identifier::Server& serverID) const
        -> UniqueQueue<OTNymID>& = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;
};
struct Pair : virtual public opentxs::api::client::Pair {
    virtual void init() noexcept = 0;

    virtual ~Pair() = default;
};
struct UI : virtual public opentxs::api::client::UI {
    virtual auto ActivateUICallback(const Identifier& widget) const noexcept
        -> void = 0;
    virtual auto RegisterUICallback(
        const Identifier& widget,
        const SimpleCallback& cb) const noexcept -> void = 0;

    virtual ~UI() = default;
};
}  // namespace opentxs::api::client::internal

namespace opentxs::factory
{
auto Activity(
    const api::internal::Core& api,
    const api::client::Contacts& contact) -> api::client::internal::Activity*;
auto BlockchainAPI(
    const api::client::internal::Manager& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args) noexcept -> std::unique_ptr<api::client::Blockchain>;
auto ClientManager(
    const api::internal::Context& parent,
    Flag& running,
    const ArgList& args,
    const api::Settings& config,
    const api::Crypto& crypto,
    const network::zeromq::Context& context,
    const std::string& dataFolder,
    const int instance) -> api::client::internal::Manager*;
auto ContactAPI(const api::client::internal::Manager& api)
    -> api::client::internal::Contacts*;
auto FactoryAPIClient(const api::client::internal::Manager& api)
    -> api::internal::Factory*;
auto Issuer(
    const api::Wallet& wallet,
    const identifier::Nym& nymID,
    const proto::Issuer& serialized) -> api::client::Issuer*;
auto Issuer(
    const api::Wallet& wallet,
    const identifier::Nym& nymID,
    const identifier::Nym& issuerID) -> api::client::Issuer*;
auto OTX(
    const Flag& running,
    const api::client::internal::Manager& api,
    OTClient& otclient,
    const ContextLockCallback& lockCallback) -> api::client::OTX*;
auto PairAPI(const Flag& running, const api::client::internal::Manager& client)
    -> api::client::internal::Pair*;
auto ServerAction(
    const api::client::internal::Manager& api,
    const ContextLockCallback& lockCallback) -> api::client::ServerAction*;
auto UI(
    const api::client::internal::Manager& api,
    const Flag& running
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::unique_ptr<api::client::internal::UI>;
auto Wallet(const api::client::internal::Manager& client) -> api::Wallet*;
auto Workflow(
    const api::internal::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contact) -> api::client::Workflow*;
}  // namespace opentxs::factory
