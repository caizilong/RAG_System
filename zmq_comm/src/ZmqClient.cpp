#include "ZmqClient.h"

namespace zmq_component {

ZmqClient::ZmqClient(const std::string& address) { setupSocket(ZMQ_REQ, address); }

void ZmqClient::sendRequest(const std::string& message) {
    zmq::message_t request(message.size());
    memcpy(request.data(), message.c_str(), message.size());

    auto result = socket_->send(request, zmq::send_flags::none);
    if (!result) {
        throw ZmqCommunicationError("Send timeout");
    }
}

std::string ZmqClient::receiveResponse() {
    zmq::message_t reply;

    auto result = socket_->recv(reply, zmq::recv_flags::none);
    if (!result) {
        throw ZmqCommunicationError("Receive timeout");
    }

    return {static_cast<char*>(reply.data()), reply.size()};
}

std::string ZmqClient::request(const std::string& message) {
    sendRequest(message);
    return receiveResponse();
}

}  // namespace zmq_component
