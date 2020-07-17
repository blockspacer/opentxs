// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <string>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/core/Identifier.hpp"

namespace
{
struct Default_Identifier : public ::testing::Test {
    ot::OTIdentifier identifier_;

    Default_Identifier()
        : identifier_(ot::Identifier::Factory())
    {
    }
};
}  // namespace

TEST_F(Default_Identifier, default_accessors)
{
    ASSERT_EQ(identifier_->data(), nullptr);
    ASSERT_EQ(identifier_->size(), 0);
}

TEST_F(Default_Identifier, serialize_empty)
{
    const auto str = identifier_->str();
    const auto recovered = ot::Identifier::Factory(str);

    EXPECT_EQ(identifier_, recovered);
}
