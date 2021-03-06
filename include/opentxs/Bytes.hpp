// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BYTES_HPP
#define OPENTXS_BYTES_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace opentxs
{
template <typename ViewType>
class ViewWrapper
{
public:
    operator bool() const noexcept { return valid(); }

    auto bytes() const noexcept -> const std::byte*
    {
        return static_cast<const std::byte*>(data());
    }
    auto data() const noexcept -> const void* { return view_.data(); }
    auto get() const noexcept -> const ViewType& { return view_; }
    auto size() const noexcept { return view_.size(); }
    auto valid() const noexcept { return (nullptr != data()) && (0 != size()); }

    auto get() noexcept -> ViewType& { return view_; }

    ViewWrapper(ViewType&& view) noexcept
        : view_(std::move(view))
    {
    }
    ViewWrapper() noexcept = default;
    ViewWrapper(const ViewWrapper&) = delete;
    ViewWrapper(ViewWrapper&&) = default;
    ViewWrapper& operator=(const ViewWrapper&) = delete;
    ViewWrapper& operator=(ViewWrapper&&) = default;
    virtual ~ViewWrapper() = default;

private:
    ViewType view_{};
};

template <typename ViewType, typename MutexType, typename LockType>
class ProtectedView : virtual public ViewWrapper<ViewType>
{
public:
    using DestructCallback = std::function<void()>;

    ProtectedView(
        ViewType&& view,
        MutexType& mutex,
        DestructCallback cb = {}) noexcept
        : ViewWrapper<ViewType>(std::move(view))
        , lock_(std::make_unique<LockType>(mutex))
        , cb_(cb)
    {
    }
    ProtectedView() noexcept = default;
    ProtectedView(const ProtectedView&) = delete;
    ProtectedView(ProtectedView&&) = default;
    ProtectedView& operator=(const ProtectedView&) = delete;
    ProtectedView& operator=(ProtectedView&&) = default;
    ~ProtectedView()
    {
        lock_.reset();

        if (cb_) { cb_(); }
    }

private:
    std::unique_ptr<const LockType> lock_{};
    DestructCallback cb_{};
};

class WritableView
{
public:
    OPENTXS_EXPORT operator void*() const noexcept { return data(); }
    OPENTXS_EXPORT operator std::size_t() const noexcept { return size(); }
    OPENTXS_EXPORT operator bool() const noexcept { return valid(); }

    template <typename DesiredType>
    OPENTXS_EXPORT auto as() const noexcept -> DesiredType*
    {
        return static_cast<DesiredType*>(data_);
    }

    OPENTXS_EXPORT auto data() const noexcept -> void* { return data_; }
    OPENTXS_EXPORT auto size() const noexcept -> std::size_t { return size_; }
    OPENTXS_EXPORT auto valid() const noexcept -> bool
    {
        return (nullptr != data_) && (0 != size_);
    }
    OPENTXS_EXPORT auto valid(const std::size_t size) const noexcept -> bool
    {
        return (nullptr != data_) && (size == size_);
    }

    OPENTXS_EXPORT WritableView(void* data, const std::size_t size) noexcept
        : data_(data)
        , size_(size)
    {
    }
    OPENTXS_EXPORT WritableView() noexcept
        : WritableView(nullptr, 0)
    {
    }
    OPENTXS_EXPORT WritableView(WritableView&& rhs) noexcept
        : data_(std::move(rhs.data_))
        , size_(std::move(rhs.size_))
    {
    }

    OPENTXS_EXPORT ~WritableView() = default;

private:
    void* data_;
    std::size_t size_;

    WritableView(const WritableView&) = delete;
    WritableView& operator=(WritableView&&) = delete;
};

using AllocateOutput = std::function<WritableView(const std::size_t)>;
using ReadView = std::string_view;
using Space = std::vector<std::byte>;
using Digest = std::function<
    bool(const std::uint32_t, const ReadView, const AllocateOutput)>;

OPENTXS_EXPORT auto preallocated(const std::size_t size, void* out) noexcept
    -> AllocateOutput;
OPENTXS_EXPORT auto reader(const WritableView& in) noexcept -> ReadView;
OPENTXS_EXPORT auto reader(const Space& in) noexcept -> ReadView;
OPENTXS_EXPORT auto space(const std::size_t size) noexcept -> Space;
OPENTXS_EXPORT auto space(const ReadView bytes) noexcept -> Space;
OPENTXS_EXPORT auto writer(std::string& in) noexcept -> AllocateOutput;
OPENTXS_EXPORT auto writer(Space& in) noexcept -> AllocateOutput;
}  // namespace opentxs
#endif
