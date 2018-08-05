// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP
#define OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP

#include "Internal.hpp"

namespace opentxs::api::implementation
{
class Wallet : virtual public opentxs::api::Wallet, Lockable
{
public:
    SharedAccount Account(
        const std::string& dataFolder,
        const Identifier& accountID) const override;
    OTIdentifier AccountPartialMatch(const std::string& hint) const override;
    ExclusiveAccount CreateAccount(
        const std::string& dataFolder,
        const Identifier& ownerNymID,
        const Identifier& notaryID,
        const Identifier& instrumentDefinitionID,
        const opentxs::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash) const override;
    bool DeleteAccount(
        const std::string& dataFolder,
        const Identifier& accountID) const override;
    SharedAccount IssuerAccount(
        const std::string& dataFolder,
        const Identifier& unitID) const override;
    ExclusiveAccount mutable_Account(
        const std::string& dataFolder,
        const Identifier& accountID) const override;
    bool UpdateAccount(
        const Identifier& accountID,
        const opentxs::ServerContext& context,
        const String& serialized) const override;
    bool ImportAccount(
        const std::string& dataFolder,
        std::unique_ptr<opentxs::Account>& imported) const override;
    std::shared_ptr<const opentxs::Context> Context(
        const Identifier& notaryID,
        const Identifier& clientNymID) const override;
    std::shared_ptr<const opentxs::ClientContext> ClientContext(
        const Identifier& localNymID,
        const Identifier& remoteNymID) const override;
    std::shared_ptr<const opentxs::ServerContext> ServerContext(
        const Identifier& localNymID,
        const Identifier& remoteID) const override;
    Editor<opentxs::Context> mutable_Context(
        const Identifier& notaryID,
        const Identifier& clientNymID) const override;
    Editor<opentxs::ClientContext> mutable_ClientContext(
        const Identifier& localNymID,
        const Identifier& remoteNymID) const override;
    Editor<opentxs::ServerContext> mutable_ServerContext(
        const Identifier& localNymID,
        const Identifier& remoteID) const override;
    std::set<OTIdentifier> IssuerList(const Identifier& nymID) const override;
    std::shared_ptr<const api::client::Issuer> Issuer(
        const Identifier& nymID,
        const Identifier& issuerID) const override;
    Editor<api::client::Issuer> mutable_Issuer(
        const Identifier& nymID,
        const Identifier& issuerID) const override;
    bool IsLocalNym(const std::string& id) const override;
    std::size_t LocalNymCount() const override;
    std::set<OTIdentifier> LocalNyms() const override;
    ConstNym Nym(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstNym Nym(const proto::CredentialIndex& nym) const override;
    ConstNym Nym(
        const std::string& dataFolder,
        const NymParameters& nymParameters,
        const proto::ContactItemType type = proto::CITEMTYPE_ERROR,
        const std::string name = "") const override;
    NymData mutable_Nym(const Identifier& id) const override;
    std::unique_ptr<const opentxs::NymFile> Nymfile(
        const std::string& dataFolder,
        const Identifier& id,
        const OTPasswordData& reason) const override;
    Editor<opentxs::NymFile> mutable_Nymfile(
        const std::string& dataFolder,
        const Identifier& id,
        const OTPasswordData& reason) const override;
    ConstNym NymByIDPartialMatch(const std::string& partialId) const override;
    ObjectList NymList() const override;
    bool NymNameByIndex(const std::size_t index, String& name) const override;
    std::shared_ptr<proto::PeerReply> PeerReply(
        const Identifier& nym,
        const Identifier& reply,
        const StorageBox& box) const override;
    bool PeerReplyComplete(
        const Identifier& nym,
        const Identifier& replyOrRequest) const override;
    bool PeerReplyCreate(
        const Identifier& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const override;
    bool PeerReplyCreateRollback(
        const Identifier& nym,
        const Identifier& request,
        const Identifier& reply) const override;
    ObjectList PeerReplySent(const Identifier& nym) const override;
    ObjectList PeerReplyIncoming(const Identifier& nym) const override;
    ObjectList PeerReplyFinished(const Identifier& nym) const override;
    ObjectList PeerReplyProcessed(const Identifier& nym) const override;
    bool PeerReplyReceive(const Identifier& nym, const PeerObject& reply)
        const override;
    std::shared_ptr<proto::PeerRequest> PeerRequest(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time) const override;
    bool PeerRequestComplete(const Identifier& nym, const Identifier& reply)
        const override;
    bool PeerRequestCreate(
        const Identifier& nym,
        const proto::PeerRequest& request) const override;
    bool PeerRequestCreateRollback(
        const Identifier& nym,
        const Identifier& request) const override;
    bool PeerRequestDelete(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box) const override;
    ObjectList PeerRequestSent(const Identifier& nym) const override;
    ObjectList PeerRequestIncoming(const Identifier& nym) const override;
    ObjectList PeerRequestFinished(const Identifier& nym) const override;
    ObjectList PeerRequestProcessed(const Identifier& nym) const override;
    bool PeerRequestReceive(const Identifier& nym, const PeerObject& request)
        const override;
    bool PeerRequestUpdate(
        const Identifier& nym,
        const Identifier& request,
        const StorageBox& box) const override;
    bool RemoveServer(const Identifier& id) const override;
    bool RemoveUnitDefinition(const Identifier& id) const override;
    ConstServerContract Server(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstServerContract Server(
        const proto::ServerContract& contract) const override;
    ConstServerContract Server(
        const std::string& nymid,
        const std::string& name,
        const std::string& terms,
        const std::list<ServerContract::Endpoint>& endpoints) const override;
    ObjectList ServerList() const override;
    bool SetNymAlias(const Identifier& id, const std::string& alias)
        const override;
    bool SetServerAlias(const Identifier& id, const std::string& alias)
        const override;
    bool SetUnitDefinitionAlias(const Identifier& id, const std::string& alias)
        const override;
    ObjectList UnitDefinitionList() const override;
    const ConstUnitDefinition UnitDefinition(
        const Identifier& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const override;
    ConstUnitDefinition UnitDefinition(
        const proto::UnitDefinition& contract) const override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t& power,
        const std::string& fraction) const override;
    ConstUnitDefinition UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms) const override;
    proto::ContactItemType CurrencyTypeBasedOnUnitType(
        const Identifier& contractID) const override;

    bool LoadCredential(
        const std::string& id,
        std::shared_ptr<proto::Credential>& credential) const override;
    bool SaveCredential(const proto::Credential& credential) const override;

    ~Wallet() = default;

private:
    using AccountLock =
        std::pair<std::shared_mutex, std::unique_ptr<opentxs::Account>>;
    using AccountMap = std::map<OTIdentifier, AccountLock>;
    using NymLock = std::pair<std::mutex, std::shared_ptr<opentxs::Nym>>;
    using NymMap = std::map<std::string, NymLock>;
    using ServerMap =
        std::map<std::string, std::shared_ptr<opentxs::ServerContract>>;
    using UnitMap =
        std::map<std::string, std::shared_ptr<opentxs::UnitDefinition>>;
    using ContextID = std::pair<std::string, std::string>;
    using ContextMap = std::map<ContextID, std::shared_ptr<opentxs::Context>>;
    using IssuerID = std::pair<OTIdentifier, OTIdentifier>;
    using IssuerLock =
        std::pair<std::mutex, std::shared_ptr<api::client::Issuer>>;
    using IssuerMap = std::map<IssuerID, IssuerLock>;

    friend opentxs::Factory;

    static const std::map<std::string, proto::ContactItemType> unit_of_account_;

    const int instance_{0};
    const api::Native& ot_;
    mutable AccountMap account_map_;
    mutable NymMap nym_map_;
    mutable ServerMap server_map_;
    mutable UnitMap unit_map_;
    mutable ContextMap context_map_;
    mutable IssuerMap issuer_map_;
    mutable std::mutex account_map_lock_;
    mutable std::mutex nym_map_lock_;
    mutable std::mutex server_map_lock_;
    mutable std::mutex unit_map_lock_;
    mutable std::mutex context_map_lock_;
    mutable std::mutex issuer_map_lock_;
    mutable std::mutex peer_map_lock_;
    mutable std::map<std::string, std::mutex> peer_lock_;
    mutable std::mutex nymfile_map_lock_;
    mutable std::map<OTIdentifier, std::mutex> nymfile_lock_;
    OTZMQPublishSocket account_publisher_;
    OTZMQPublishSocket issuer_publisher_;
    OTZMQPublishSocket nym_publisher_;
    OTZMQPublishSocket server_publisher_;
    OTZMQRequestSocket dht_nym_requester_;
    OTZMQRequestSocket dht_server_requester_;
    OTZMQRequestSocket dht_unit_requester_;

    std::string account_alias(const std::string& accountID) const;
    opentxs::Account* account_factory(
        const std::string& dataFolder,
        const Identifier& accountID,
        const std::string& alias,
        const std::string& serialized) const;
    proto::ContactItemType extract_unit(const Identifier& contractID) const;
    proto::ContactItemType extract_unit(
        const opentxs::UnitDefinition& contract) const;
    bool load_legacy_account(
        const std::string& dataFolder,
        const Identifier& accountID,
        const Identifier& notaryID,
        const eLock& lock,
        AccountLock& row) const;
    Editor<opentxs::NymFile> mutable_nymfile(
        const std::string& dataFolder,
        const std::shared_ptr<const opentxs::Nym>& targetNym,
        const std::shared_ptr<const opentxs::Nym>& signerNym,
        const Identifier& id,
        const OTPasswordData& reason) const;
    std::mutex& nymfile_lock(const Identifier& nymID) const;
    std::mutex& peer_lock(const std::string& nymID) const;
    void publish_server(const Identifier& id) const;
    void save(
        const std::string id,
        std::unique_ptr<opentxs::Account>& in,
        eLock& lock,
        bool success) const;
    void save(opentxs::Context* context) const;
    void save(const Lock& lock, api::client::Issuer* in) const;
    void save(NymData* nymData, const Lock& lock) const;
    void save(opentxs::NymFile* nym, const Lock& lock) const;
    bool SaveCredentialIDs(const opentxs::Nym& nym) const;
    std::shared_ptr<const opentxs::Nym> signer_nym(const Identifier& id) const;

    /* Throws std::out_of_range for missing accounts */
    AccountLock& account(
        const Lock& lock,
        const std::string& dataFolder,
        const Identifier& accountID,
        const bool create) const;
    std::shared_ptr<opentxs::Context> context(
        const Identifier& localNymID,
        const Identifier& remoteNymID) const;
    IssuerLock& issuer(
        const Identifier& nymID,
        const Identifier& issuerID,
        const bool create) const;

    /**   Save an instantiated server contract to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated ServerContract object
     */
    ConstServerContract Server(std::unique_ptr<ServerContract>& contract) const;
    OTIdentifier ServerToNym(OTIdentifier& serverID) const;

    /**   Save an instantiated unit definition to storage and add to internal
     *    map.
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] contract the instantiated UnitDefinition object
     */
    ConstUnitDefinition UnitDefinition(
        std::unique_ptr<opentxs::UnitDefinition>& contract) const;

    Wallet(
        const int instance,
        const api::Native& ot,
        const opentxs::network::zeromq::Context& zmq);
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_CLIENT_IMPLEMENTATION_WALLET_HPP
