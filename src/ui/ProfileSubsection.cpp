// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "ui/ProfileSubsection.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <type_traits>

#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "ui/Combined.hpp"
#include "ui/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ProfileSubsection::"

namespace opentxs::factory
{
auto ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    ui::implementation::CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept -> std::shared_ptr<ui::implementation::ProfileSectionRowInternal>
{
    using ReturnType = ui::implementation::ProfileSubsection;

    return std::make_shared<ReturnType>(
        parent,
        api,
        rowID,
        key,
        custom
#if OT_QT
        ,
        qt
#endif
    );
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ProfileSubsection::ProfileSubsection(
    const ProfileSectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ProfileSectionRowID& rowID,
    const ProfileSectionSortKey& key,
    CustomData& custom
#if OT_QT
    ,
    const bool qt
#endif
    ) noexcept
    : Combined(
          api,
          parent.NymID(),
          parent.WidgetID(),
          parent,
          rowID,
          key
#if OT_QT
          ,
          qt
#endif
      )
{
    init();
    startup_.reset(new std::thread(
        &ProfileSubsection::startup,
        this,
        extract_custom<opentxs::ContactGroup>(custom)));

    OT_ASSERT(startup_)
}

auto ProfileSubsection::AddItem(
    const std::string& value,
    const bool primary,
    const bool active) const noexcept -> bool
{
    return parent_.AddClaim(row_id_.second, value, primary, active);
}

auto ProfileSubsection::construct_row(
    const ProfileSubsectionRowID& id,
    const ProfileSubsectionSortKey& index,
    CustomData& custom) const noexcept -> void*
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id, factory::ProfileItemWidget(*this, api_, id, index, custom));

    return it->second.get();
}

auto ProfileSubsection::Delete(const std::string& claimID) const noexcept
    -> bool
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.Delete();
}

auto ProfileSubsection::Name(const std::string& lang) const noexcept
    -> std::string
{
    return proto::TranslateItemType(row_id_.second, lang);
}

auto ProfileSubsection::process_group(
    const opentxs::ContactGroup& group) noexcept
    -> std::set<ProfileSubsectionRowID>
{
    OT_ASSERT(row_id_.second == group.Type())

    std::set<ProfileSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new opentxs::ContactItem(*claim)};
        add_item(id, sort_key(id), custom);
        active.emplace(id);
    }

    return active;
}

void ProfileSubsection::reindex(
    const ProfileSectionSortKey&,
    CustomData& custom) noexcept
{
    delete_inactive(
        process_group(extract_custom<opentxs::ContactGroup>(custom)));
}

auto ProfileSubsection::SetActive(const std::string& claimID, const bool active)
    const noexcept -> bool
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetActive(active);
}

auto ProfileSubsection::SetPrimary(
    const std::string& claimID,
    const bool primary) const noexcept -> bool
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetPrimary(primary);
}

auto ProfileSubsection::SetValue(
    const std::string& claimID,
    const std::string& value) const noexcept -> bool
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetValue(value);
}

auto ProfileSubsection::sort_key(const ProfileSubsectionRowID) const noexcept
    -> int
{
    return static_cast<int>(items_.size());
}

void ProfileSubsection::startup(const opentxs::ContactGroup group) noexcept
{
    process_group(group);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
