// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Server.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/LegacySymmetric.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include "core/InternalCore.hpp"

#include <irrxml/irrXML.hpp>
#include <sodium/crypto_box.h>
#include <sys/types.h>

#include <atomic>
#include <array>
#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "NymFile.hpp"

#define NYMFILE_VERSION "1.1"

//#define OT_METHOD "opentxs::NymFile::"

namespace opentxs
{
internal::NymFile* Factory::NymFile(
    std::shared_ptr<const Nym> targetNym,
    std::shared_ptr<const Nym> signerNym,
    const std::string& dataFolder)
{
    return new implementation::NymFile(targetNym, signerNym, dataFolder);
}
}  // namespace opentxs

namespace opentxs::implementation
{
NymFile::NymFile(
    std::shared_ptr<const Nym> targetNym,
    std::shared_ptr<const Nym> signerNym,
    const std::string& dataFolder)
    : target_nym_{targetNym}
    , signer_nym_{signerNym}
    , m_lUsageCredits(0)
    , m_bMarkForDeletion(false)
    , m_strNymFile()
    , m_strVersion(NYMFILE_VERSION)
    , m_strDescription("")
    , data_folder_(dataFolder)
    , m_mapInboxHash()
    , m_mapOutboxHash()
    , m_dequeOutpayments()
    , m_setAccounts()
{
}

/// a payments message is a form of transaction, transported via Nymbox
/// Though the parameter is a reference (forcing you to pass a real object),
/// the Nym DOES take ownership of the object. Therefore it MUST be allocated
/// on the heap, NOT the stack, or you will corrupt memory with this call.
void NymFile::AddOutpayments(Message& theMessage)
{
    eLock lock(shared_lock_);

    m_dequeOutpayments.push_front(&theMessage);
}

void NymFile::ClearAll()
{
    eLock lock(shared_lock_);

    m_mapInboxHash.clear();
    m_mapOutboxHash.clear();
    m_setAccounts.clear();
    ClearOutpayments();
}

void NymFile::ClearOutpayments()
{
    while (GetOutpaymentsCount() > 0) RemoveOutpaymentsByIndex(0, true);
}

bool NymFile::CompareID(const Identifier& rhs) const
{
    sLock lock(shared_lock_);

    return rhs == target_nym_->ID();
}

bool NymFile::DeserializeNymFile(
    const String& strNym,
    bool& converted,
    String::Map* pMapCredentials,
    String* pstrReason,
    const OTPassword* pImportPassword)
{
    sLock lock(shared_lock_);

    return deserialize_nymfile(
        lock, strNym, converted, pMapCredentials, pstrReason, pImportPassword);
}

template <typename T>
bool NymFile::deserialize_nymfile(
    const T& lock,
    const String& strNym,
    bool& converted,
    String::Map* pMapCredentials,
    String* pstrReason,
    const OTPassword* pImportPassword)
{
    OT_ASSERT(verify_lock(lock));

    bool bSuccess = false;
    bool convert = false;
    converted = false;
    //?    ClearAll();  // Since we are loading everything up... (credentials
    // are NOT
    // cleared here. See note in OTPseudonym::ClearAll.)
    OTStringXML strNymXML(strNym);  // todo optimize
    irr::io::IrrXMLReader* xml = irr::io::createIrrXMLReader(strNymXML);
    OT_ASSERT(nullptr != xml);
    std::unique_ptr<irr::io::IrrXMLReader> theCleanup(xml);
    const auto serverMode = OT::App().ServerMode();
    OTIdentifier serverID{Identifier::Factory()};

    if (serverMode) { serverID = OT::App().Server().ID(); }

    // parse the file until end reached
    while (xml && xml->read()) {

        // strings for storing the data that we want to read out of the file
        //
        switch (xml->getNodeType()) {
            case irr::io::EXN_NONE:
            case irr::io::EXN_TEXT:
            case irr::io::EXN_COMMENT:
            case irr::io::EXN_ELEMENT_END:
            case irr::io::EXN_CDATA:
                break;
            case irr::io::EXN_ELEMENT: {
                const String strNodeName = xml->getNodeName();

                if (strNodeName.Compare("nymData")) {
                    m_strVersion = xml->getAttributeValue("version");
                    const String UserNymID = xml->getAttributeValue("nymID");

                    // Server-side only...
                    String strCredits = xml->getAttributeValue("usageCredits");

                    if (strCredits.GetLength() > 0)
                        m_lUsageCredits = strCredits.ToLong();
                    else
                        m_lUsageCredits = 0;  // This is the default anyway, but
                                              // just being safe...

                    if (UserNymID.GetLength())
                        otLog3 << "\nLoading user, version: " << m_strVersion
                               << " NymID:\n"
                               << UserNymID << "\n";
                    bSuccess = true;
                    convert = (String("1.0") == m_strVersion);

                    if (convert) {
                        otErr << __FUNCTION__
                              << ": Converting nymfile with version "
                              << m_strVersion << std::endl;
                    } else {
                        otWarn << __FUNCTION__
                               << ": Not converting nymfile because version is "
                               << m_strVersion << std::endl;
                    }
                } else if (strNodeName.Compare("nymIDSource")) {
                    // noop
                } else if (strNodeName.Compare("inboxHashItem")) {
                    const String strAccountID =
                        xml->getAttributeValue("accountID");
                    const String strHashValue =
                        xml->getAttributeValue("hashValue");

                    otLog3 << "\nInboxHash is " << strHashValue
                           << " for Account ID: " << strAccountID << "\n";

                    // Make sure now that I've loaded this InboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID.Exists() && strHashValue.Exists()) {
                        auto pID = Identifier::Factory(strHashValue);
                        OT_ASSERT(!pID->empty())
                        m_mapInboxHash.emplace(strAccountID.Get(), pID);
                    }
                } else if (strNodeName.Compare("outboxHashItem")) {
                    const String strAccountID =
                        xml->getAttributeValue("accountID");
                    const String strHashValue =
                        xml->getAttributeValue("hashValue");

                    otLog3 << "\nOutboxHash is " << strHashValue
                           << " for Account ID: " << strAccountID << "\n";

                    // Make sure now that I've loaded this OutboxHash, to add it
                    // to
                    // my
                    // internal map so that it is available for future lookups.
                    //
                    if (strAccountID.Exists() && strHashValue.Exists()) {
                        OTIdentifier pID = Identifier::Factory(strHashValue);
                        OT_ASSERT(!pID->empty())
                        m_mapOutboxHash.emplace(strAccountID.Get(), pID);
                    }
                } else if (strNodeName.Compare("MARKED_FOR_DELETION")) {
                    m_bMarkForDeletion = true;
                    otLog3 << "This nym has been MARKED_FOR_DELETION (at some "
                              "point prior.)\n";
                } else if (strNodeName.Compare("ownsAssetAcct")) {
                    String strID = xml->getAttributeValue("ID");

                    if (strID.Exists()) {
                        m_setAccounts.insert(strID.Get());
                        otLog3 << "This nym has an asset account with the ID: "
                               << strID << "\n";
                    } else
                        otLog3
                            << "This nym MISSING asset account ID when loading "
                               "nym record.\n";
                } else if (strNodeName.Compare("outpaymentsMessage")) {
                    Armored armorMail;
                    String strMessage;

                    xml->read();

                    if (irr::io::EXN_TEXT == xml->getNodeType()) {
                        String strNodeData = xml->getNodeData();

                        // Sometimes the XML reads up the data with a prepended
                        // newline.
                        // This screws up my own objects which expect a
                        // consistent
                        // in/out
                        // So I'm checking here for that prepended newline, and
                        // removing it.
                        char cNewline;
                        if (strNodeData.Exists() &&
                            strNodeData.GetLength() > 2 &&
                            strNodeData.At(0, cNewline)) {
                            if ('\n' == cNewline)
                                armorMail.Set(strNodeData.Get() + 1);
                            else
                                armorMail.Set(strNodeData.Get());

                            if (armorMail.GetLength() > 2) {
                                armorMail.GetString(
                                    strMessage,
                                    true);  // linebreaks == true.

                                if (strMessage.GetLength() > 2) {
                                    Message* pMessage =
                                        new Message{data_folder_};

                                    OT_ASSERT(nullptr != pMessage);

                                    if (pMessage->LoadContractFromString(
                                            strMessage)) {
                                        m_dequeOutpayments.push_back(
                                            pMessage);  // takes ownership
                                    } else {
                                        delete pMessage;
                                    }
                                }
                            }
                        }  // strNodeData
                    }      // EXN_TEXT
                }          // outpayments message
                else {
                    // unknown element type
                    otErr << "Unknown element type in " << __FUNCTION__ << ": "
                          << xml->getNodeName() << "\n";
                    bSuccess = false;
                }
                break;
            }
            default: {
                otLog5 << "Unknown XML type in " << __FUNCTION__ << ": "
                       << xml->getNodeName() << "\n";
                break;
            }
        }  // switch
    }      // while

    if (converted) { m_strVersion = "1.1"; }

    return bSuccess;
}

void NymFile::DisplayStatistics(String& strOutput) const
{
    sLock lock(shared_lock_);
    strOutput.Concatenate(
        "Source for ID:\n%s\n", target_nym_->Source().asString().Get());
    strOutput.Concatenate("Description: %s\n\n", m_strDescription.Get());
    strOutput.Concatenate("%s", "\n");
    strOutput.Concatenate(
        "==>      Name: %s   %s\n",
        target_nym_->Alias().c_str(),
        m_bMarkForDeletion ? "(MARKED FOR DELETION)" : "");
    strOutput.Concatenate("      Version: %s\n", m_strVersion.Get());
    strOutput.Concatenate(
        "Outpayments count: %" PRI_SIZE "\n", m_dequeOutpayments.size());

    String theStringID(target_nym_->ID());
    strOutput.Concatenate("Nym ID: %s\n", theStringID.Get());
}

bool NymFile::GetHash(
    const mapOfIdentifiers& the_map,
    const std::string& str_id,
    Identifier& theOutput) const  // client-side
{
    sLock lock(shared_lock_);

    bool bRetVal =
        false;  // default is false: "No, I didn't find a hash for that id."
    theOutput.Release();

    // The Pseudonym has a map of its recent hashes, one for each server
    // (nymbox) or account (inbox/outbox).
    // For Server Bob, with this Pseudonym, I might have hash lkjsd987345lkj.
    // For but Server Alice, I might have hash 98345jkherkjghdf98gy.
    // (Same Nym, but different hash for each server, as well as inbox/outbox
    // hashes for each asset acct.)
    //
    // So let's loop through all the hashes I have, and if the ID on the map
    // passed in
    // matches the [server|acct] ID that was passed in, then return TRUE.
    //
    for (const auto& it : the_map) {
        if (str_id == it.first) {
            // The call has succeeded
            bRetVal = true;
            theOutput.SetString(it.second->str());
            break;
        }
    }

    return bRetVal;
}

bool NymFile::GetInboxHash(
    const std::string& acct_id,
    Identifier& theOutput) const  // client-side
{
    return GetHash(m_mapInboxHash, acct_id, theOutput);
}

bool NymFile::GetOutboxHash(
    const std::string& acct_id,
    Identifier& theOutput) const  // client-side
{
    return GetHash(m_mapOutboxHash, acct_id, theOutput);
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
Message* NymFile::GetOutpaymentsByIndex(std::int32_t nIndex) const
{
    sLock lock(shared_lock_);
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size())) {

        return nullptr;
    }

    return m_dequeOutpayments.at(nIndex);
}

Message* NymFile::GetOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    std::unique_ptr<OTPayment>* pReturnPayment /*=nullptr*/,
    std::int32_t* pnReturnIndex /*=nullptr*/) const
{
    if (nullptr != pnReturnIndex) { *pnReturnIndex = -1; }

    const std::int32_t nCount = GetOutpaymentsCount();

    for (std::int32_t nIndex = 0; nIndex < nCount; ++nIndex) {
        Message* pMsg = m_dequeOutpayments.at(nIndex);
        OT_ASSERT(nullptr != pMsg);
        String strPayment;
        std::unique_ptr<OTPayment> payment;
        std::unique_ptr<OTPayment>& pPayment(
            nullptr == pReturnPayment ? payment : *pReturnPayment);

        // There isn't any encrypted envelope this time, since it's my
        // outPayments box.
        //
        if (pMsg->m_ascPayload.Exists() &&
            pMsg->m_ascPayload.GetString(strPayment) && strPayment.Exists()) {
            pPayment.reset(new OTPayment(data_folder_, strPayment));

            // Let's see if it's the cheque we're looking for...
            //
            if (pPayment && pPayment->IsValid()) {
                if (pPayment->SetTempValues()) {
                    if (pPayment->HasTransactionNum(lTransNum)) {

                        if (nullptr != pnReturnIndex) {
                            *pnReturnIndex = nIndex;
                        }

                        return pMsg;
                    }
                }
            }
        }
    }
    return nullptr;
}

/// return the number of payments items available for this Nym.
std::int32_t NymFile::GetOutpaymentsCount() const
{
    return static_cast<std::int32_t>(m_dequeOutpayments.size());
}

bool NymFile::LoadSignedNymFile()
{
    sLock lock(shared_lock_);

    return load_signed_nymfile(lock);
}

template <typename T>
bool NymFile::load_signed_nymfile(const T& lock)
{
    OT_ASSERT(verify_lock(lock));

    // Get the Nym's ID in string form
    String nymID(target_nym_->ID());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    OTSignedFile theNymFile(data_folder_, OTFolders::Nym(), nymID);

    if (!theNymFile.LoadFile()) {
        otWarn << __FUNCTION__ << ": Failed loading a signed nymfile: " << nymID
               << std::endl;

        return false;
    }

    // We verify:
    //
    // 1. That the file even exists and loads.
    // 2. That the local subdir and filename match the versions inside the file.
    // 3. That the signature matches for the signer nym who was passed in.
    //
    if (!theNymFile.VerifyFile()) {
        otErr << __FUNCTION__ << ": Failed verifying nymfile: " << nymID
              << "\n\n";

        return false;
    }

    const auto& publicSignKey = signer_nym_->GetPublicSignKey();

    if (!theNymFile.VerifyWithKey(publicSignKey)) {
        otErr << __FUNCTION__
              << ": Failed verifying signature on nymfile: " << nymID
              << "\n Signer Nym ID: " << signer_nym_->ID().str() << "\n";

        return false;
    }

    otInfo << "Loaded and verified signed nymfile. Reading from string...\n";

    if (1 > theNymFile.GetFilePayload().GetLength()) {
        const auto lLength = theNymFile.GetFilePayload().GetLength();

        otErr << __FUNCTION__ << ": Bad length (" << lLength
              << ") while loading nymfile: " << nymID << "\n";
    }

    bool converted = false;
    const bool loaded = deserialize_nymfile(
        lock,
        theNymFile.GetFilePayload(),
        converted,
        nullptr,
        nullptr,
        nullptr);

    if (!loaded) { return false; }

    if (converted) {
        // This will ensure that none of the old tags will be present the next
        // time this nym is loaded.
        // Converting a nym more than once is likely to cause sync issues.
        save_signed_nymfile(lock);
    }

    return true;
}

// Sometimes for testing I need to clear out all the transaction numbers from a
// nym. So I added this method to make such a thing easy to do.
void NymFile::RemoveAllNumbers(const String* pstrNotaryID)
{
    std::list<mapOfIdentifiers::iterator> listOfInboxHash;
    std::list<mapOfIdentifiers::iterator> listOfOutboxHash;

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapInboxHash.begin()); it != m_mapInboxHash.end(); ++it) {
        listOfInboxHash.push_back(it);
    }

    // This is mapped to acct_id, not notary_id.
    // (So we just wipe them all.)
    for (auto it(m_mapOutboxHash.begin()); it != m_mapOutboxHash.end(); ++it) {
        listOfOutboxHash.push_back(it);
    }

    while (!listOfInboxHash.empty()) {
        m_mapInboxHash.erase(listOfInboxHash.back());
        listOfInboxHash.pop_back();
    }
    while (!listOfOutboxHash.empty()) {
        m_mapOutboxHash.erase(listOfOutboxHash.back());
        listOfOutboxHash.pop_back();
    }
}

// if this function returns false, outpayments index was bad.
bool NymFile::RemoveOutpaymentsByIndex(
    const std::int32_t nIndex,
    bool bDeleteIt)
{
    const std::uint32_t uIndex = nIndex;

    // Out of bounds.
    if (m_dequeOutpayments.empty() || (nIndex < 0) ||
        (uIndex >= m_dequeOutpayments.size())) {
        otErr << __FUNCTION__
              << ": Error: Index out of bounds: signed: " << nIndex
              << " unsigned: " << uIndex << " (deque size is "
              << m_dequeOutpayments.size() << ").\n";
        return false;
    }

    Message* pMessage = m_dequeOutpayments.at(nIndex);
    OT_ASSERT(nullptr != pMessage);

    m_dequeOutpayments.erase(m_dequeOutpayments.begin() + uIndex);

    if (bDeleteIt) delete pMessage;

    return true;
}

bool NymFile::RemoveOutpaymentsByTransNum(
    const std::int64_t lTransNum,
    bool bDeleteIt /*=true*/)
{
    std::int32_t nReturnIndex = -1;

    Message* pMsg =
        this->GetOutpaymentsByTransNum(lTransNum, nullptr, &nReturnIndex);
    const std::uint32_t uIndex = nReturnIndex;

    if ((nullptr != pMsg) && (nReturnIndex > (-1)) &&
        (uIndex < m_dequeOutpayments.size())) {
        m_dequeOutpayments.erase(m_dequeOutpayments.begin() + uIndex);
        if (bDeleteIt) delete pMsg;
        return true;
    }
    return false;
}

// ** ResyncWithServer **
//
// Not for normal use! (Since you should never get out of sync with the server
// in the first place.)
// However, in testing, or if some bug messes up some data, or whatever, and you
// absolutely need to
// re-sync with a server, and you trust that server not to lie to you, then this
// function will do the trick.
// NOTE: Before calling this, you need to do a getNymbox() to download the
// latest Nymbox, and you need to do
// a registerNym() to download the server's copy of your Nym. You then
// need to load that Nymbox from
// local storage, and you need to load the server's message Nym out of the
// registerNymResponse reply, so that
// you can pass both of those objects into this function, which must assume that
// those pieces were already done
// just prior to this call.
//
bool NymFile::ResyncWithServer(
    const Ledger& theNymbox,
    const Nym& theMessageNym)
{
    eLock lock(shared_lock_);

    bool bSuccess = true;
    const Identifier& theNotaryID = theNymbox.GetRealNotaryID();
    const String strNotaryID(theNotaryID);
    const String strNymID(target_nym_->ID());

    auto context = OT::App().Wallet().mutable_ServerContext(
        target_nym_->ID(), theNotaryID);

    // Remove all issued, transaction, and tentative numbers for a specific
    // server ID,
    // as well as all acknowledgedNums, and the highest transaction number for
    // that notaryID,
    // from *this nym. Leave our record of the highest trans num received from
    // that server,
    // since we will want to just keep it when re-syncing. (Server doesn't store
    // that anyway.)
    //
    RemoveAllNumbers(&strNotaryID);

    std::set<TransactionNumber> setTransNumbers;

    // loop through theNymbox and add Tentative numbers to *this based on each
    // successNotice in the Nymbox. This way, when the notices are processed,
    // they will succeed because the Nym will believe he was expecting them.
    for (auto& it : theNymbox.GetTransactionMap()) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);
        //        OTString strTransaction(*pTransaction);
        //        otErr << "TRANSACTION CONTENTS:\n%s\n", strTransaction.Get());

        // (a new; ALREADY just added transaction number.)
        if ((OTTransaction::successNotice !=
             pTransaction->GetType()))  // if !successNotice
            continue;

        const std::int64_t lNum =
            pTransaction->GetReferenceToNum();  // successNotice is inRefTo the
                                                // new transaction # that should
                                                // be on my tentative list.

        // Add to list of tentatively-being-added numbers.
        if (!context.It().AddTentativeNumber(lNum)) {
            otErr << "OTPseudonym::ResyncWithServer: Failed trying to add "
                     "TentativeNum ("
                  << lNum << ") onto *this nym: " << strNymID
                  << ", for server: " << strNotaryID << "\n";
            bSuccess = false;
        } else
            otWarn << "OTPseudonym::ResyncWithServer: Added TentativeNum ("
                   << lNum << ") onto *this nym: " << strNymID
                   << ", for server: " << strNotaryID << " \n";
        // There's no "else insert to setTransNumbers" here, like the other two
        // blocks above.
        // Why not? Because setTransNumbers is for updating the Highest Trans
        // Num record on this Nym,
        // and the Tentative Numbers aren't figured into that record until AFTER
        // they are accepted
        // from the Nymbox. So I leave them out, since this function is
        // basically setting us up to
        // successfully process the Nymbox, which will then naturally update the
        // highest num record
        // based on the tentatives, as it's removing them from the tentative
        // list and adding them to
        // the "available" transaction list (and issued.)
    }

    std::set<TransactionNumber> notUsed;
    context.It().UpdateHighest(setTransNumbers, notUsed, notUsed);

    return (save_signed_nymfile(lock) && bSuccess);
}

// Save the Pseudonym to a string...
bool NymFile::SerializeNymFile(String& output) const
{
    sLock lock(shared_lock_);

    return serialize_nymfile(lock, output);
}

template <typename T>
bool NymFile::serialize_nymfile(const T& lock, String& strNym) const
{
    OT_ASSERT(verify_lock(lock));

    Tag tag("nymData");

    String nymID(target_nym_->ID());

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("nymID", nymID.Get());

    if (m_lUsageCredits != 0)
        tag.add_attribute("usageCredits", formatLong(m_lUsageCredits));

    target_nym_->SerializeNymIDSource(tag);

    // When you delete a Nym, it just marks it.
    // Actual deletion occurs during maintenance sweep
    // (targeting marked nyms...)
    //
    if (m_bMarkForDeletion) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS NYM HAS BEEN MARKED "
            "FOR DELETION AT ITS OWN REQUEST");
    }

    if (!(m_dequeOutpayments.empty())) {
        for (std::uint32_t i = 0; i < m_dequeOutpayments.size(); i++) {
            Message* pMessage = m_dequeOutpayments.at(i);
            OT_ASSERT(nullptr != pMessage);

            String strOutpayments(*pMessage);

            Armored ascOutpayments;

            if (strOutpayments.Exists())
                ascOutpayments.SetString(strOutpayments);

            if (ascOutpayments.Exists()) {
                tag.add_tag("outpaymentsMessage", ascOutpayments.Get());
            }
        }
    }

    // These are used on the server side.
    // (That's why you don't see the server ID saved here.)
    //
    if (!(m_setAccounts.empty())) {
        for (auto& it : m_setAccounts) {
            std::string strID(it);
            TagPtr pTag(new Tag("ownsAssetAcct"));
            pTag->add_attribute("ID", strID);
            tag.add_tag(pTag);
        }
    }

    // client-side
    for (auto& it : m_mapInboxHash) {
        std::string strAcctID = it.first;
        const Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const String strHash(theID);
            TagPtr pTag(new Tag("inboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash.Get());
            tag.add_tag(pTag);
        }
    }  // for

    // client-side
    for (auto& it : m_mapOutboxHash) {
        std::string strAcctID = it.first;
        const Identifier& theID = it.second;

        if ((strAcctID.size() > 0) && !theID.empty()) {
            const String strHash(theID);
            TagPtr pTag(new Tag("outboxHashItem"));
            pTag->add_attribute("accountID", strAcctID);
            pTag->add_attribute("hashValue", strHash.Get());
            tag.add_tag(pTag);
        }
    }  // for

    std::string str_result;
    tag.output(str_result);

    strNym.Concatenate("%s", str_result.c_str());

    return true;
}

bool NymFile::SerializeNymFile(const char* szFoldername, const char* szFilename)
{
    OT_ASSERT(nullptr != szFoldername);
    OT_ASSERT(nullptr != szFilename);

    sLock lock(shared_lock_);

    String strNym;
    serialize_nymfile(lock, strNym);

    bool bSaved = OTDB::StorePlainString(
        strNym.Get(), data_folder_, szFoldername, szFilename, "", "");
    if (!bSaved)
        otErr << __FUNCTION__ << ": Error saving file: " << szFoldername
              << Log::PathSeparator() << szFilename << "\n";

    return bSaved;
}

bool NymFile::SaveSignedNymFile()
{
    eLock lock(shared_lock_);

    return save_signed_nymfile(lock);
}

template <typename T>
bool NymFile::save_signed_nymfile(const T& lock)
{
    OT_ASSERT(verify_lock(lock));

    // Get the Nym's ID in string form
    String strNymID(target_nym_->ID());

    // Create an OTSignedFile object, giving it the filename (the ID) and the
    // local directory ("nyms")
    OTSignedFile theNymFile(data_folder_, OTFolders::Nym().Get(), strNymID);
    theNymFile.GetFilename(m_strNymFile);

    otInfo << "Saving nym to: " << m_strNymFile << "\n";

    // First we save this nym to a string...
    // Specifically, the file payload string on the OTSignedFile object.
    serialize_nymfile(lock, theNymFile.GetFilePayload());

    // Now the OTSignedFile contains the path, the filename, AND the
    // contents of the Nym itself, saved to a string inside the OTSignedFile
    // object.

    const auto& privateSignKey = signer_nym_->GetPrivateSignKey();

    if (theNymFile.SignWithKey(privateSignKey) && theNymFile.SaveContract()) {
        const bool bSaved = theNymFile.SaveFile();

        if (!bSaved) {
            otErr << __FUNCTION__
                  << ": Failed while calling theNymFile.SaveFile() for Nym "
                  << strNymID << " using Signer Nym " << signer_nym_->ID().str()
                  << "\n";
        }

        return bSaved;
    } else {
        otErr << __FUNCTION__
              << ": Failed trying to sign and save NymFile for Nym " << strNymID
              << " using Signer Nym " << signer_nym_->ID().str() << "\n";
    }

    return false;
}

bool NymFile::SetHash(
    mapOfIdentifiers& the_map,
    const std::string& str_id,
    const Identifier& theInput)  // client-side
{
    bool bSuccess = false;

    auto find_it = the_map.find(str_id);

    if (the_map.end() != find_it)  // found something for that str_id
    {
        // The call has succeeded
        the_map.erase(find_it);
        OTIdentifier pID = Identifier::Factory(theInput);
        OT_ASSERT(!pID->empty())
        the_map.emplace(str_id, pID);
        bSuccess = true;
    }

    // If I didn't find it in the list above (whether the list is empty or
    // not....)
    // that means it does not exist. (So create it.)
    //
    if (!bSuccess) {
        OTIdentifier pID = Identifier::Factory(theInput);
        OT_ASSERT(!pID->empty())
        the_map.emplace(str_id, pID);
    }
    //    if (bSuccess)
    //    {
    //        SaveSignedNymFile(SIGNER_NYM);
    //    }

    return bSuccess;
}

bool NymFile::SetInboxHash(
    const std::string& acct_id,
    const Identifier& theInput)  // client-side
{
    eLock lock(shared_lock_);

    return SetHash(m_mapInboxHash, acct_id, theInput);
}

bool NymFile::SetOutboxHash(
    const std::string& acct_id,
    const Identifier& theInput)  // client-side
{
    eLock lock(shared_lock_);

    return SetHash(m_mapOutboxHash, acct_id, theInput);
}

NymFile::~NymFile() { ClearAll(); }
}  // namespace opentxs::implementation
