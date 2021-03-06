// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "ui/ContactListItem.hpp"  // IWYU pragma: associated

#include <locale>
#include <memory>

#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ContactListItem::"

namespace opentxs::factory
{
auto ContactListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::ContactListRowInternal>
{
    using ReturnType = ui::implementation::ContactListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, key);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactListItem::ContactListItem(
    const ContactListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ContactListRowID& rowID,
    const ContactListSortKey& key) noexcept
    : ContactListItemRow(parent, api, rowID, true)
    , key_(key)
{
}

auto ContactListItem::ContactID() const noexcept -> std::string
{
    return row_id_->str();
}

auto ContactListItem::DisplayName() const noexcept -> std::string
{
    Lock lock(lock_);

    return key_;
}

auto ContactListItem::ImageURI() const noexcept -> std::string
{
    // TODO

    return {};
}

#if OT_QT
QVariant ContactListItem::qt_data([[maybe_unused]] const int column, int role)
    const noexcept
{
    switch (role) {
        case Qt::DisplayRole: {
            return DisplayName().c_str();
        }
        case Qt::DecorationRole: {
            // TODO render ImageURI into a QPixmap
            return {};
        }
        case ContactListQt::ContactIDRole: {
            return ContactID().c_str();
        }
        case ContactListQt::SectionRole: {
            return Section().c_str();
        }
        default: {
            return {};
        }
    }
}
#endif

void ContactListItem::reindex(
    const ContactListSortKey& key,
    CustomData&) noexcept
{
    Lock lock(lock_);
    key_ = key;
}

auto ContactListItem::Section() const noexcept -> std::string
{
    Lock lock(lock_);

    if (row_id_ == parent_.ID()) { return {"ME"}; }

    if (key_.empty()) { return {" "}; }

    std::locale locale;
    std::string output{" "};
    output[0] = std::toupper(key_[0], locale);

    return output;
}
}  // namespace opentxs::ui::implementation
