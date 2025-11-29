#include "ZmqServer.h"

namespace zmq_component {

ZmqServer::ZmqServer(const std::string &address) { setupSocket(ZMQ_REP, address); }

std::string ZmqServer::receive() {
    zmq::message_t request;

    auto result = socket_->recv(request, zmq::recv_flags::none);
    if (!result) {
        throw ZmqCommunicationError("Receive timeout");
    }

    return {static_cast<char *>(request.data()), request.size()};
}

void ZmqServer::send(const std::string &response) {
    zmq::message_t reply(response.size());
    memcpy(reply.data(), response.c_str(), response.size());

    auto result = socket_->send(reply, zmq::send_flags::none);
    if (!result) {
        throw ZmqCommunicationError("Send timeout");
    }
}

}  // namespace zmq_component