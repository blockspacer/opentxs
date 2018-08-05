// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/api/client/Blockchain.hpp"
#endif
#include "opentxs/api/client/Cash.hpp"
#include "opentxs/api/client/Client.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "api/Scheduler.hpp"
#include "internal/api/Internal.hpp"
#include "InternalClient.hpp"

#include <set>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "Client.hpp"

namespace opentxs
{
api::client::internal::Client* Factory::Client(
    const Flag& running,
    const api::Settings& config,
    const api::Crypto& crypto,
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds,
#endif
    const api::Legacy& legacy,
    const api::storage::Storage& storage,
    const api::Wallet& wallet,
    const network::zeromq::Context& context,
    const int instance)
{
    return new api::client::implementation::Client(
        running,
        config,
        crypto,
#if OT_CRYPTO_WITH_BIP39
        seeds,
#endif
        legacy,
        storage,
        wallet,
        context,
        instance);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Client::Client(
    const Flag& running,
    const api::Settings& config,
    const api::Crypto& crypto,
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds,
#endif
    const api::Legacy& legacy,
    const api::storage::Storage& storage,
    const api::Wallet& wallet,
    const opentxs::network::zeromq::Context& context,
    const int instance)
    : Scheduler(running)
    , running_(running)
    , wallet_(wallet)
    , storage_(storage)
    , crypto_(crypto)
#if OT_CRYPTO_WITH_BIP39
    , seeds_(seeds)
#endif
    , legacy_(legacy)
    , config_(config)
    , zmq_context_{context}
    , instance_{instance}
    , activity_(nullptr)
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , blockchain_(nullptr)
#endif
    , cash_(nullptr)
    , contacts_(nullptr)
    , pair_(nullptr)
    , server_action_(nullptr)
    , sync_(nullptr)
    , workflow_(nullptr)
    , dht_{opentxs::Factory::Dht(
          instance_,
          false,
          config_,
          wallet_,
          context,
          nym_publish_interval_,
          nym_refresh_interval_,
          server_publish_interval_,
          server_refresh_interval_,
          unit_publish_interval_,
          unit_refresh_interval_)}
    , zeromq_(nullptr)
    , factory_(nullptr)
    , identity_(nullptr)
    , ot_api_(nullptr)
    , otapi_exec_(nullptr)
    , lock_()
    , map_lock_()
    , context_locks_()
{
    OT_ASSERT(dht_)

    Init();
}

const api::client::Activity& Client::Activity() const
{
    OT_ASSERT(activity_)

    return *activity_;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
const api::client::Blockchain& Client::Blockchain() const
{
    OT_ASSERT(blockchain_)

    return *blockchain_;
}
#endif

void Client::Cleanup()
{
    pair_.reset();
    ui_.reset();
    SwigWrap::client_ = nullptr;
    sync_.reset();
    server_action_.reset();
    cash_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
    workflow_.reset();
#if OT_CRYPTO_SUPPORTED_KEY_HD
    blockchain_.reset();
#endif
    activity_.reset();
    contacts_.reset();
    zeromq_.reset();
    factory_.reset();
    identity_.reset();
}

const api::client::Contacts& Client::Contacts() const
{
    OT_ASSERT(contacts_)

    return *contacts_;
}

const api::network::Dht& Client::DHT() const
{
    OT_ASSERT(dht_)

    return *dht_;
}

const api::Factory& Client::Factory() const
{
    OT_ASSERT(factory_)

    return *factory_;
}

std::recursive_mutex& Client::get_lock(const ContextID context) const
{
    opentxs::Lock lock(map_lock_);

    return context_locks_[context];
}

void Client::Init()
{
    otLog3 << "\n\nWelcome to Open Transactions -- version " << Log::Version()
           << "\n";

    Init_Identity();  // No dependencies
    Init_Factory();   // No dependencies
    Init_ZMQ();
    Init_Contacts();  // Requires Init_Factory()
    Init_Activity();  // Requires Init_Contacts(), Init_Factory()
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Init_Blockchain();  // Requires Init_Activity()
#endif
    Init_Workflow();      // Requires Init_Activity(), Init_Contacts()
    Init_OldClientAPI();  // Requires Init_Activity(), Init_Contacts(),
                          // Init_Workflow(), Init_Factory(), Init_Identity()
                          // Init_ZMQ();
    Init_Cash();          // Requires Init_OldClientAPI()
    Init_ServerAction();  // Requires Init_OldClientAPI(), Init_Workflow()
    Init_Sync();          // Requires Init_OldClientAPI(), Init_Contacts(),
                          // Init_Workflow(), Init_ServerAction()
    Init_UI();    // Requires Init_Activity(), Init_Sync(), Init_Workflow(),
                  // Init_Contacts(), Init_ZMQ()
    Init_Pair();  // Requires Init_OldClientAPI(), Init_Sync(),
                  // Init_ServerAction
}

void Client::Init_Activity()
{
    OT_ASSERT(contacts_)
    OT_ASSERT(factory_)

    activity_.reset(opentxs::Factory::Activity(
        storage_, *contacts_, *factory_, legacy_, wallet_, zmq_context_));

    OT_ASSERT(activity_)
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
void Client::Init_Blockchain()
{
    OT_ASSERT(activity_)

    blockchain_.reset(opentxs::Factory::Blockchain(
        *activity_, crypto_, seeds_, storage_, wallet_));

    OT_ASSERT(blockchain_)
}
#endif

void Client::Init_Cash()
{
    cash_.reset(opentxs::Factory::Cash(legacy_));

    OT_ASSERT(cash_);
}

void Client::Init_Contacts()
{
    OT_ASSERT(factory_)

    contacts_.reset(
        opentxs::Factory::Contacts(storage_, *factory_, wallet_, zmq_context_));

    OT_ASSERT(contacts_)
}

void Client::Init_Factory()
{
    factory_.reset(opentxs::Factory::FactoryAPI(
#if OT_CRYPTO_WITH_BIP39
        seeds_
#endif
        ));

    OT_ASSERT(factory_)
}

void Client::Init_Identity()
{
    identity_.reset(opentxs::Factory::Identity(wallet_));
}

void Client::Init_OldClientAPI()
{
    OT_ASSERT(activity_);
    OT_ASSERT(contacts_);
    OT_ASSERT(factory_)
    OT_ASSERT(identity_)
    OT_ASSERT(workflow_);
    OT_ASSERT(zeromq_);

    ot_api_.reset(new OT_API(
        *activity_,
        *this,
        config_,
        *contacts_,
        crypto_,
        *factory_,
#if OT_CRYPTO_WITH_BIP39
        seeds_,
#endif
        *identity_,
        legacy_,
        storage_,
        wallet_,
        *workflow_,
        *zeromq_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(ot_api_);

    otapi_exec_.reset(new OTAPI_Exec(
        *activity_,
        config_,
        *contacts_,
        crypto_,
        *factory_,
        *identity_,
        legacy_,
        wallet_,
        *zeromq_,
        *ot_api_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(otapi_exec_);

    if (0 == instance_) {
        SwigWrap::client_ = this;

        OT_ASSERT(SwigWrap::client_)
    }
}

void Client::Init_Pair()
{
    OT_ASSERT(ot_api_)
    OT_ASSERT(otapi_exec_)
    OT_ASSERT(sync_)
    OT_ASSERT(server_action_)

    pair_.reset(opentxs::Factory::Pair(
        running_,
        *sync_,
        *server_action_,
        wallet_,
        legacy_,
        *ot_api_,
        *otapi_exec_,
        zmq_context_));

    OT_ASSERT(pair_);
}

void Client::Init_ServerAction()
{
    OT_ASSERT(ot_api_)
    OT_ASSERT(otapi_exec_)
    OT_ASSERT(workflow_)

    server_action_.reset(opentxs::Factory::ServerAction(
        *ot_api_,
        *otapi_exec_,
        wallet_,
        *workflow_,
        legacy_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(server_action_)
}

void Client::Init_Sync()
{
    OT_ASSERT(contacts_)
    OT_ASSERT(ot_api_)
    OT_ASSERT(otapi_exec_)
    OT_ASSERT(workflow_)

    sync_.reset(opentxs::Factory::Sync(
        running_,
        *ot_api_,
        *otapi_exec_,
        *contacts_,
        config_,
        *this,
        legacy_,
        wallet_,
        *workflow_,
        crypto_.Encode(),
        storage_,
        zmq_context_,
        std::bind(&Client::get_lock, this, std::placeholders::_1)));

    OT_ASSERT(sync_);
}

void Client::Init_UI()
{
    OT_ASSERT(activity_)
    OT_ASSERT(contacts_)
    OT_ASSERT(sync_)
    OT_ASSERT(workflow_)
    OT_ASSERT(zeromq_)

    ui_.reset(opentxs::Factory::UI(
        *sync_,
        wallet_,
        *workflow_,
        *zeromq_,
        storage_,
        *activity_,
        *contacts_,
        legacy_,
        zmq_context_,
        running_));

    OT_ASSERT(ui_);
}

void Client::Init_Workflow()
{
    OT_ASSERT(activity_)
    OT_ASSERT(contacts_)

    workflow_.reset(opentxs::Factory::Workflow(
        *activity_, *contacts_, legacy_, storage_, zmq_context_));

    OT_ASSERT(workflow_)
}

void Client::Init_ZMQ()
{
    zeromq_.reset(opentxs::Factory::ZMQ(zmq_context_, config_, running_));
}

const OTAPI_Exec& Client::Exec(const std::string&) const
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

std::recursive_mutex& Client::Lock(
    const Identifier& nymID,
    const Identifier& serverID) const
{
    return get_lock({nymID.str(), serverID.str()});
}

const OT_API& Client::OTAPI(const std::string&) const
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

const api::client::Cash& Client::Cash() const
{
    OT_ASSERT(cash_);

    return *cash_;
}

const api::client::Pair& Client::Pair() const
{
    OT_ASSERT(pair_);

    return *pair_;
}

const api::client::ServerAction& Client::ServerAction() const
{
    OT_ASSERT(server_action_);

    return *server_action_;
}

void Client::StartActivity()
{
    OT_ASSERT(activity_)

    activity_->MigrateLegacyThreads();

    OT_ASSERT(dht_)

    Scheduler::Start(&storage_, dht_.get());
}

void Client::StartContacts()
{
    OT_ASSERT(contacts_);

    contacts_->start();
}

opentxs::OTWallet* Client::StartWallet()
{
    OT_ASSERT(ot_api_)

    const bool loaded = ot_api_->LoadWallet();

    OT_ASSERT(loaded);

    return ot_api_->GetWallet(nullptr);
}

void Client::storage_gc_hook()
{
    // if (storage_) { storage_->RunGC(); }

    storage_.RunGC();
}

const api::client::Sync& Client::Sync() const
{
    OT_ASSERT(sync_);

    return *sync_;
}

const api::client::UI& Client::UI() const
{
    OT_ASSERT(ui_)

    return *ui_;
}

const api::client::Workflow& Client::Workflow() const
{
    OT_ASSERT(workflow_);

    return *workflow_;
}

const api::network::ZMQ& Client::ZMQ() const
{
    OT_ASSERT(zeromq_);

    return *zeromq_;
}

Client::~Client() { Cleanup(); }
}  // namespace opentxs::api::client::implementation
