#include "PCF.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

static uint32_t policyIdCounter = 20000;

PCF::PCF() : NetworkFunction(NFType::PCF, "PCF") {
    logger_.info(name_, "PCF initialized");
}

std::string PCF::createPolicy(UeId ueId, SessionId sessionId, uint32_t bitrate, uint32_t priority) {
    std::string policyId = generatePolicyId();

    PolicyRule policy;
    policy.ruleName = "Policy_" + std::to_string(ueId);
    policy.ueId = ueId;
    policy.sessionId = sessionId;
    policy.maxBitrate = bitrate;
    policy.priorityLevel = priority;
    policy.isActive = true;

    policies_[policyId] = policy;
    activePolicies_.insert(policyId);

    logPolicyCreation(policyId, ueId);

    return policyId;
}

bool PCF::updatePolicy(const std::string& policyId, uint32_t newBitrate) {
    auto it = policies_.find(policyId);
    if (it == policies_.end()) {
        logger_.warning(name_, "Policy not found: " + policyId);
        return false;
    }

    it->second.maxBitrate = newBitrate;

    logger_.info(name_, "Policy updated | ID=" + policyId + 
                       " | New Bitrate=" + std::to_string(newBitrate) + "kbps");

    return true;
}

bool PCF::removePolicy(const std::string& policyId) {
    auto it = policies_.find(policyId);
    if (it == policies_.end()) {
        logger_.warning(name_, "Policy not found: " + policyId);
        return false;
    }

    logPolicyRemoval(policyId);
    policies_.erase(it);
    activePolicies_.erase(policyId);

    return true;
}

PCF::PolicyRule* PCF::getPolicy(const std::string& policyId) {
    auto it = policies_.find(policyId);
    if (it != policies_.end()) {
        return &it->second;
    }
    return nullptr;
}

void PCF::recordChargingEvent(UeId ueId, SessionId sessionId, uint64_t bytes) {
    // Simple charging: 0.001 units per MB
    uint64_t charge = (bytes / 1000000) + 1;
    chargeRecords_[ueId] += charge;

    logger_.debug(name_, "Charging Event | UE=" + std::to_string(ueId) + 
                        " | Session=" + std::to_string(sessionId) + 
                        " | Bytes=" + std::to_string(bytes) + 
                        " | Charge=" + std::to_string(charge));
}

uint64_t PCF::getTotalCharge(UeId ueId) const {
    auto it = chargeRecords_.find(ueId);
    if (it != chargeRecords_.end()) {
        return it->second;
    }
    return 0;
}

void PCF::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::PDU_SESSION_ESTABLISHMENT_REQUEST: {
            auto pduMsg = std::dynamic_pointer_cast<PduSessionEstablishmentRequestMessage>(message);
            if (pduMsg) {
                createPolicy(message->getSourceId(), pduMsg->getSessionId(), 5000, 9);
            }
            break;
        }
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void PCF::printActivePolicies() const {
    std::cout << "\n================== PCF Active Policies ==================\n";
    std::cout << "Total Policies: " << policies_.size() << "\n";
    std::cout << "Active Policies: " << activePolicies_.size() << "\n\n";

    for (const auto& pair : policies_) {
        const auto& policy = pair.second;
        std::cout << "Policy ID: " << pair.first 
                  << " | UE=" << policy.ueId 
                  << " | Session=" << policy.sessionId 
                  << " | MaxBitrate=" << policy.maxBitrate << "kbps"
                  << " | Priority=" << policy.priorityLevel << "\n";
    }

    if (!chargeRecords_.empty()) {
        std::cout << "\nCharging Records:\n";
        for (const auto& pair : chargeRecords_) {
            std::cout << "UE " << pair.first << ": " << pair.second << " units\n";
        }
    }
    std::cout << "=========================================================\n\n";
}

std::string PCF::getPCFStatus() const {
    std::ostringstream oss;
    oss << "PCF Status:\n"
        << "  Active Policies: " << activePolicies_.size() << "\n"
        << "  Charge Records: " << chargeRecords_.size() << "\n";
    return oss.str();
}

std::string PCF::generatePolicyId() {
    return "POL_" + std::to_string(++policyIdCounter);
}

void PCF::logPolicyCreation(const std::string& policyId, UeId ueId) {
    logger_.info(name_, "Policy Created | ID=" + policyId + 
                       " | UE=" + std::to_string(ueId));
}

void PCF::logPolicyRemoval(const std::string& policyId) {
    logger_.info(name_, "Policy Removed | ID=" + policyId);
}

void PCF::start() {
    NetworkFunction::start();
    logger_.info(name_, "PCF started and ready for policy management");
}

void PCF::stop() {
    NetworkFunction::stop();
    policies_.clear();
    chargeRecords_.clear();
    activePolicies_.clear();
    logger_.info(name_, "PCF stopped");
}
