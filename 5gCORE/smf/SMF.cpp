#include "SMF.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

static uint32_t sessionIdCounter = 5000;
static uint32_t ipAddrCounter = 1;

SMF::SMF() : NetworkFunction(NFType::SMF, "SMF") {
    logger_.info(name_, "SMF initialized");
}

SessionId SMF::createPduSession(UeId ueId, const std::string& dnn, Snssai snssai) {
    SessionId sessionId = ++sessionIdCounter;

    PduSessionContext context;
    context.pduSessionId = sessionId;
    context.ueId = ueId;
    context.state = SessionState::ACTIVATING;
    context.snssai = snssai;
    context.dnn = dnn;
    context.ipv4Address = generateIpv4Address();
    context.ipv6Address = generateIpv6Address();
    context.ulTraffic = 0;
    context.dlTraffic = 0;

    pduSessions_[sessionId] = context;
    ueSessionMap_[ueId].push_back(sessionId);

    logSessionCreation(sessionId, ueId);

    return sessionId;
}

bool SMF::activatePduSession(SessionId sessionId) {
    auto it = pduSessions_.find(sessionId);
    if (it == pduSessions_.end()) {
        logger_.error(name_, "Session not found: " + std::to_string(sessionId));
        return false;
    }

    it->second.state = SessionState::ACTIVE;
    activeSessions_[sessionId] = SessionState::ACTIVE;

    logSessionActivation(sessionId);

    return true;
}

bool SMF::modifyPduSession(SessionId sessionId, const std::string& newDnn) {
    auto it = pduSessions_.find(sessionId);
    if (it == pduSessions_.end()) {
        logger_.error(name_, "Session not found: " + std::to_string(sessionId));
        return false;
    }

    if (it->second.state != SessionState::ACTIVE) {
        logger_.warning(name_, "Cannot modify inactive session: " + std::to_string(sessionId));
        return false;
    }

    it->second.state = SessionState::MODIFYING;
    it->second.dnn = newDnn;
    it->second.state = SessionState::ACTIVE;

    logger_.info(name_, "Session modified: " + std::to_string(sessionId) + 
                       " | New DNN: " + newDnn);

    return true;
}

bool SMF::releasePduSession(SessionId sessionId) {
    auto it = pduSessions_.find(sessionId);
    if (it == pduSessions_.end()) {
        logger_.warning(name_, "Session not found for release: " + std::to_string(sessionId));
        return false;
    }

    it->second.state = SessionState::DEACTIVATING;
    activeSessions_.erase(sessionId);

    logger_.debug(name_, "Session deactivating: " + std::to_string(sessionId));

    return true;
}

bool SMF::terminatePduSession(SessionId sessionId) {
    auto it = pduSessions_.find(sessionId);
    if (it == pduSessions_.end()) {
        logger_.warning(name_, "Session not found for termination: " + std::to_string(sessionId));
        return false;
    }

    UeId ueId = it->second.ueId;
    pduSessions_.erase(it);
    activeSessions_.erase(sessionId);

    // Remove from UE session map
    auto& ueSession = ueSessionMap_[ueId];
    ueSession.erase(std::remove(ueSession.begin(), ueSession.end(), sessionId), 
                    ueSession.end());

    logSessionTermination(sessionId);

    return true;
}

PduSessionContext* SMF::getPduSessionContext(SessionId sessionId) {
    auto it = pduSessions_.find(sessionId);
    if (it != pduSessions_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SMF::isSessionActive(SessionId sessionId) const {
    return activeSessions_.find(sessionId) != activeSessions_.end();
}

std::vector<SessionId> SMF::getActiveSessions(UeId ueId) const {
    std::vector<SessionId> result;
    
    auto it = ueSessionMap_.find(ueId);
    if (it != ueSessionMap_.end()) {
        for (SessionId sessionId : it->second) {
            if (isSessionActive(sessionId)) {
                result.push_back(sessionId);
            }
        }
    }

    return result;
}

void SMF::recordUplink(SessionId sessionId, uint64_t bytes) {
    auto it = pduSessions_.find(sessionId);
    if (it != pduSessions_.end()) {
        it->second.ulTraffic += bytes;
        logger_.debug(name_, "Uplink recorded: Session=" + std::to_string(sessionId) + 
                            " | Bytes=" + std::to_string(bytes));
    }
}

void SMF::recordDownlink(SessionId sessionId, uint64_t bytes) {
    auto it = pduSessions_.find(sessionId);
    if (it != pduSessions_.end()) {
        it->second.dlTraffic += bytes;
        logger_.debug(name_, "Downlink recorded: Session=" + std::to_string(sessionId) + 
                            " | Bytes=" + std::to_string(bytes));
    }
}

void SMF::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::PDU_SESSION_ESTABLISHMENT_REQUEST: {
            auto pduMsg = std::dynamic_pointer_cast<PduSessionEstablishmentRequestMessage>(message);
            if (pduMsg) {
                createPduSession(message->getSourceId(), pduMsg->getDnn(), 1);
                activatePduSession(pduMsg->getSessionId());
            }
            break;
        }
        case MessageType::DATA_TRANSFER: {
            auto dataMsg = std::dynamic_pointer_cast<DataTransferMessage>(message);
            if (dataMsg) {
                recordUplink(dataMsg->getSessionId(), dataMsg->getDataSize());
            }
            break;
        }
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void SMF::printActiveSessions() const {
    std::cout << "\n================== SMF Active Sessions ==================\n";
    std::cout << "Total Sessions: " << pduSessions_.size() << "\n";
    std::cout << "Active Sessions: " << activeSessions_.size() << "\n\n";

    for (const auto& pair : pduSessions_) {
        const auto& session = pair.second;
        std::cout << "Session ID: " << session.pduSessionId 
                  << " | UE: " << session.ueId 
                  << " | DNN: " << session.dnn 
                  << " | IPv4: " << session.ipv4Address 
                  << " | UL: " << session.ulTraffic << "B"
                  << " | DL: " << session.dlTraffic << "B\n";
    }
    std::cout << "========================================================\n\n";
}

std::string SMF::getSMFStatus() const {
    std::ostringstream oss;
    oss << "SMF Status:\n"
        << "  Total Sessions: " << pduSessions_.size() << "\n"
        << "  Active Sessions: " << activeSessions_.size() << "\n";
    return oss.str();
}

std::string SMF::generateIpv4Address() {
    std::ostringstream oss;
    oss << "10.0." << (ipAddrCounter / 256) << "." << (ipAddrCounter % 256);
    ipAddrCounter++;
    return oss.str();
}

std::string SMF::generateIpv6Address() {
    std::ostringstream oss;
    oss << "fd00::" << std::hex << ipAddrCounter;
    return oss.str();
}

void SMF::logSessionCreation(SessionId sessionId, UeId ueId) {
    logger_.info(name_, "PDU Session Created | ID=" + std::to_string(sessionId) + 
                       " | UE=" + std::to_string(ueId));
}

void SMF::logSessionActivation(SessionId sessionId) {
    logger_.info(name_, "PDU Session Activated | ID=" + std::to_string(sessionId));
}

void SMF::logSessionTermination(SessionId sessionId) {
    logger_.info(name_, "PDU Session Terminated | ID=" + std::to_string(sessionId));
}

void SMF::start() {
    NetworkFunction::start();
    logger_.info(name_, "SMF started and ready for session management");
}

void SMF::stop() {
    NetworkFunction::stop();
    pduSessions_.clear();
    ueSessionMap_.clear();
    activeSessions_.clear();
    logger_.info(name_, "SMF stopped");
}
