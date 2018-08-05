// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Socket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"

#include <zmq.h>

#define INPROC_PREFIX "inproc://opentxs/"
#define INSTANCE_SEPERATOR "/"
#define ENDPOINT_VERSION_1 1
#define DHT_NYM_REQUEST_ENDPOINT "dht/requestnym"
#define DHT_SERVER_REQUEST_ENDPOINT "dht/requestserver"
#define DHT_UNIT_REQUEST_ENDPOINT "dht/requestunit"
#define ACCOUNT_UPDATE_ENDPOINT "inproc://opentxs/accountupdate/1"
#define CONTACT_UPDATE_ENDPOINT "inproc://opentxs/contactupdate/1"
#define CONNECTION_STATUS_ENDPOINT "inproc://opentxs/connectionstatus/1"
#define ISSUER_UPDATE_ENDPOINT "inproc://opentxs/issuerupdate/1"
#define NYM_UPDATE_ENDPOINT "inproc://opentxs/nymupdate/1"
#define PAIR_EVENT_ENDPOINT "inproc://opentxs/pairevent/1"
#define PAIR_ENDPOINT_PREFIX "inproc://opentxs/1/pair/"
#define PENDING_BAILMENT_ENDPOINT                                              \
    "inproc://opentxs/peerrequest/pendingbailment/1"
#define SERVER_UPDATE_ENDPOINT "inproc://opentxs/serverupdate/1"
#define THREAD_UPDATE_ENDPOINT "inproc://opentxs/threadupdate/1/"
#define WIDGET_UPDATE_ENDPOINT "inproc://opentxs/ui/widgetupdate/1"
#define WORKFLOW_ACCOUNT_UPDATE_ENDPOINT                                       \
    "inproc://opentxs/ui/workflowupdate/account/1"

#define OT_METHOD "opentxs::network::zeromq::implementation::Socket::"

namespace opentxs::network::zeromq
{
const std::string Socket::AccountUpdateEndpoint{ACCOUNT_UPDATE_ENDPOINT};
const std::string Socket::ContactUpdateEndpoint{CONTACT_UPDATE_ENDPOINT};
const std::string Socket::ConnectionStatusEndpoint{CONNECTION_STATUS_ENDPOINT};
const std::string Socket::IssuerUpdateEndpoint{ISSUER_UPDATE_ENDPOINT};
const std::string Socket::NymDownloadEndpoint{NYM_UPDATE_ENDPOINT};
const std::string Socket::PairEndpointPrefix{PAIR_ENDPOINT_PREFIX};
const std::string Socket::PairEventEndpoint{PAIR_EVENT_ENDPOINT};
const std::string Socket::PendingBailmentEndpoint{PENDING_BAILMENT_ENDPOINT};
const std::string Socket::ServerUpdateEndpoint{SERVER_UPDATE_ENDPOINT};
const std::string Socket::ThreadUpdateEndpoint{THREAD_UPDATE_ENDPOINT};
const std::string Socket::WidgetUpdateEndpoint{WIDGET_UPDATE_ENDPOINT};
const std::string Socket::WorkflowAccountUpdateEndpoint{
    WORKFLOW_ACCOUNT_UPDATE_ENDPOINT};

std::string build_inproc_path(
    const std::string& path,
    const int instance,
    const int version)
{
    return std::string(INPROC_PREFIX) + std::to_string(instance) +
           INSTANCE_SEPERATOR + path + std::to_string(version);
}

std::string Socket::GetDhtRequestNymEndpoint(const int instance)
{
    return build_inproc_path(
        DHT_NYM_REQUEST_ENDPOINT, instance, ENDPOINT_VERSION_1);
}

std::string Socket::GetDhtRequestServerEndpoint(const int instance)
{
    return build_inproc_path(
        DHT_SERVER_REQUEST_ENDPOINT, instance, ENDPOINT_VERSION_1);
}

std::string Socket::GetDhtRequestUnitEndpoint(const int instance)
{
    return build_inproc_path(
        DHT_UNIT_REQUEST_ENDPOINT, instance, ENDPOINT_VERSION_1);
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
const std::map<SocketType, int> Socket::types_{
    {SocketType::Request, ZMQ_REQ},
    {SocketType::Reply, ZMQ_REP},
    {SocketType::Publish, ZMQ_PUB},
    {SocketType::Subscribe, ZMQ_SUB},
    {SocketType::Pull, ZMQ_PULL},
    {SocketType::Push, ZMQ_PUSH},
    {SocketType::Pair, ZMQ_PAIR},
    {SocketType::Dealer, ZMQ_DEALER},
    {SocketType::Router, ZMQ_ROUTER},
};

Socket::Socket(const zeromq::Context& context, const SocketType type)
    : context_(context)
    , socket_(zmq_socket(context, types_.at(type)))
    , type_(type)
{
    OT_ASSERT(nullptr != socket_);
}

Socket::operator void*() const { return socket_; }

bool Socket::apply_timeouts(const Lock& lock) const
{
    OT_ASSERT(nullptr != socket_)
    OT_ASSERT(verify_lock(lock))

    auto set = zmq_setsockopt(socket_, ZMQ_LINGER, &linger_, sizeof(linger_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_LINGER"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket_, ZMQ_SNDTIMEO, &send_timeout_, sizeof(send_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_SNDTIMEO"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket_, ZMQ_RCVTIMEO, &receive_timeout_, sizeof(receive_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_RCVTIMEO"
              << std::endl;

        return false;
    }

    return true;
}

bool Socket::bind(const Lock& lock, const std::string& endpoint) const
{
    apply_timeouts(lock);

    return (0 == zmq_bind(socket_, endpoint.c_str()));
}

bool Socket::connect(const Lock& lock, const std::string& endpoint) const
{
    apply_timeouts(lock);

    return (0 == zmq_connect(socket_, endpoint.c_str()));
}

bool Socket::Close() const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    return (0 == zmq_close(socket_));
}

bool Socket::set_socks_proxy(const std::string& proxy) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    const auto set =
        zmq_setsockopt(socket_, ZMQ_SOCKS_PROXY, proxy.data(), proxy.size());

    return (0 == set);
}

bool Socket::SetTimeouts(
    const std::chrono::milliseconds& linger,
    const std::chrono::milliseconds& send,
    const std::chrono::milliseconds& receive) const
{
    Lock lock(lock_);
    linger_ = linger.count();
    send_timeout_ = send.count();
    receive_timeout_ = receive.count();

    return apply_timeouts(lock);
}

bool Socket::start_client(const Lock& lock, const std::string& endpoint) const
{
    OT_ASSERT(nullptr != socket_);

    if (false == connect(lock, endpoint)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to connect to "
              << endpoint << std::endl;

        return false;
    }

    return true;
}

SocketType Socket::Type() const { return type_; }

Socket::~Socket()
{
    Lock lock(lock_);

    if (nullptr != socket_) { zmq_close(socket_); }
}
}  // namespace opentxs::network::zeromq::implementation
