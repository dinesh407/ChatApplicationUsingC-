#include "UPF.hpp"
#include <iostream>
#include <algorithm>

UPF::UPF() : NetworkFunction(NFType::UPF, "UPF"),
             totalUplinkTraffic_(0), totalDownlinkTraffic_(0) {
    logger_.info(name_, "UPF initialized");
}

void UPF::attachPduSession(SessionId sessionId, UeId ueId) {
    if (attachedSessions_.find(sessionId) != attachedSessions_.end()) {
        logger_.warning(name_, "Session already attached: " + std::to_string(sessionId));
        return;
    }

    SessionMetrics metrics;
    metrics.sessionId = sessionId;
    metrics.ueId = ueId;
    metrics.uplinkBytes = 0;
    metrics.downlinkBytes = 0;
    metrics.qosRate = 1000;  // Default 1 Mbps
    metrics.isAttached = true;

    attachedSessions_[sessionId] = metrics;

    logger_.info(name_, "PDU Session attached | Session=" + std::to_string(sessionId) + 
                       " | UE=" + std::to_string(ueId));
}

void UPF::detachPduSession(SessionId sessionId) {
    auto it = attachedSessions_.find(sessionId);
    if (it == attachedSessions_.end()) {
        logger_.warning(name_, "Session not found: " + std::to_string(sessionId));
        return;
    }

    attachedSessions_.erase(it);
    logger_.info(name_, "PDU Session detached | Session=" + std::to_string(sessionId));
}

void UPF::forwardUplinkPacket(SessionId sessionId, uint32_t packetSize) {
    auto it = attachedSessions_.find(sessionId);
    if (it == attachedSessions_.end()) {
        logger_.warning(name_, "Cannot forward: Session not found - " + 
                               std::to_string(sessionId));
        return;
    }

    it->second.uplinkBytes += packetSize;
    totalUplinkTraffic_ += packetSize;

    logPacketForwarding(sessionId, true, packetSize);
}

void UPF::forwardDownlinkPacket(SessionId sessionId, uint32_t packetSize) {
    auto it = attachedSessions_.find(sessionId);
    if (it == attachedSessions_.end()) {
        logger_.warning(name_, "Cannot forward: Session not found - " + 
                               std::to_string(sessionId));
        return;
    }

    it->second.downlinkBytes += packetSize;
    totalDownlinkTraffic_ += packetSize;

    logPacketForwarding(sessionId, false, packetSize);
}

void UPF::setQoS(SessionId sessionId, uint32_t bitrate) {
    auto it = attachedSessions_.find(sessionId);
    if (it == attachedSessions_.end()) {
        logger_.warning(name_, "Cannot set QoS: Session not found - " + 
                               std::to_string(sessionId));
        return;
    }

    it->second.qosRate = bitrate;
    logger_.debug(name_, "QoS configured | Session=" + std::to_string(sessionId) + 
                        " | Rate=" + std::to_string(bitrate) + "kbps");
}

uint32_t UPF::getQoS(SessionId sessionId) const {
    auto it = attachedSessions_.find(sessionId);
    if (it != attachedSessions_.end()) {
        return it->second.qosRate;
    }
    return 0;
}

uint64_t UPF::getSessionUplinkTraffic(SessionId sessionId) const {
    auto it = attachedSessions_.find(sessionId);
    if (it != attachedSessions_.end()) {
        return it->second.uplinkBytes;
    }
    return 0;
}

uint64_t UPF::getSessionDownlinkTraffic(SessionId sessionId) const {
    auto it = attachedSessions_.find(sessionId);
    if (it != attachedSessions_.end()) {
        return it->second.downlinkBytes;
    }
    return 0;
}

void UPF::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::DATA_TRANSFER: {
            auto dataMsg = std::dynamic_pointer_cast<DataTransferMessage>(message);
            if (dataMsg) {
                forwardUplinkPacket(dataMsg->getSessionId(), dataMsg->getDataSize());
            }
            break;
        }
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void UPF::printSessionMetrics() const {
    std::cout << "\n================== UPF Session Metrics ==================\n";
    std::cout << "Attached Sessions: " << attachedSessions_.size() << "\n";
    std::cout << "Total UL Traffic: " << totalUplinkTraffic_ << " bytes\n";
    std::cout << "Total DL Traffic: " << totalDownlinkTraffic_ << " bytes\n";
    std::cout << "Total Traffic: " << (totalUplinkTraffic_ + totalDownlinkTraffic_) << " bytes\n\n";

    for (const auto& pair : attachedSessions_) {
        const auto& metrics = pair.second;
        std::cout << "Session " << metrics.sessionId 
                  << " | UE=" << metrics.ueId 
                  << " | UL=" << metrics.uplinkBytes << "B" 
                  << " | DL=" << metrics.downlinkBytes << "B"
                  << " | QoS=" << metrics.qosRate << "kbps\n";
    }
    std::cout << "=========================================================\n\n";
}

std::string UPF::getUPFStatus() const {
    std::ostringstream oss;
    oss << "UPF Status:\n"
        << "  Attached Sessions: " << attachedSessions_.size() << "\n"
        << "  Total UL Traffic: " << totalUplinkTraffic_ << " bytes\n"
        << "  Total DL Traffic: " << totalDownlinkTraffic_ << " bytes\n";
    return oss.str();
}

void UPF::logPacketForwarding(SessionId sessionId, bool isUplink, uint32_t size) {
    std::string direction = isUplink ? "UL" : "DL";
    logger_.debug(name_, "Packet Forward | " + direction + " | Session=" + 
                        std::to_string(sessionId) + " | Size=" + std::to_string(size) + "B");
}

void UPF::start() {
    NetworkFunction::start();
    logger_.info(name_, "UPF started and ready for packet forwarding");
}

void UPF::stop() {
    NetworkFunction::stop();
    attachedSessions_.clear();
    logger_.info(name_, "UPF stopped");
}
