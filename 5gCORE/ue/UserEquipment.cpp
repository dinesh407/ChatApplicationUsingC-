#include "UserEquipment.hpp"
#include <iostream>
#include <sstream>

UserEquipment::UserEquipment(UeId ueId, Imsi imsi, Imei imei, const std::string& phoneNumber)
    : ueId_(ueId), imsi_(imsi), imei_(imei), phoneNumber_(phoneNumber),
      state_(UeState::IDLE), connectedGnb_(0), currentSessionId_(0),
      totalUlData_(0), totalDlData_(0) {
    logger_.info("UE", "Creating UE: ID=" + std::to_string(ueId) + 
                       ", IMSI=" + std::to_string(imsi));
}

void UserEquipment::setState(UeState newState) {
    if (state_ == newState) return;
    
    std::string oldState = stateToString(state_);
    std::string newStateStr = stateToString(newState);
    
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " state transition: " + 
                        oldState + " -> " + newStateStr);
    
    state_ = newState;
}

void UserEquipment::attachToGnb(GnbId gnbId) {
    connectedGnb_ = gnbId;
    setState(UeState::CONNECTED);
    attachTime_ = std::chrono::system_clock::now();
    
    logger_.info("UE", "UE " + std::to_string(ueId_) + " attached to gNodeB " + 
                       std::to_string(gnbId));
}

void UserEquipment::detachFromGnb() {
    logger_.info("UE", "UE " + std::to_string(ueId_) + " detached from gNodeB " + 
                       std::to_string(connectedGnb_));
    
    connectedGnb_ = 0;
    setState(UeState::DISCONNECTED);
}

void UserEquipment::registerAtCore() {
    setState(UeState::REGISTERED);
    registrationTime_ = std::chrono::system_clock::now();
    
    logger_.info("UE", "UE " + std::to_string(ueId_) + " registered at core network");
}

void UserEquipment::deregister() {
    setState(UeState::IDLE);
    currentSessionId_ = 0;
    
    logger_.info("UE", "UE " + std::to_string(ueId_) + " deregistered from core network");
}

void UserEquipment::createSession(SessionId sessionId) {
    currentSessionId_ = sessionId;
    sessionUlData_[sessionId] = 0;
    sessionDlData_[sessionId] = 0;
    
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " created session " + 
                        std::to_string(sessionId));
}

void UserEquipment::activateSession(SessionId sessionId) {
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " activated session " + 
                        std::to_string(sessionId));
}

void UserEquipment::deactivateSession(SessionId sessionId) {
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " deactivated session " + 
                        std::to_string(sessionId));
}

void UserEquipment::terminateSession(SessionId sessionId) {
    if (currentSessionId_ == sessionId) {
        currentSessionId_ = 0;
    }
    
    sessionUlData_.erase(sessionId);
    sessionDlData_.erase(sessionId);
    
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " terminated session " + 
                        std::to_string(sessionId));
}

void UserEquipment::sendData(SessionId sessionId, uint32_t bytes) {
    totalUlData_ += bytes;
    sessionUlData_[sessionId] += bytes;
    
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " sent " + 
                        std::to_string(bytes) + " bytes on session " + 
                        std::to_string(sessionId));
}

void UserEquipment::receiveData(SessionId sessionId, uint32_t bytes) {
    totalDlData_ += bytes;
    sessionDlData_[sessionId] += bytes;
    
    logger_.debug("UE", "UE " + std::to_string(ueId_) + " received " + 
                        std::to_string(bytes) + " bytes on session " + 
                        std::to_string(sessionId));
}

std::shared_ptr<Message> UserEquipment::createAttachRequest() {
    return std::make_shared<AttachRequestMessage>(ueId_, imsi_, imei_);
}

std::shared_ptr<Message> UserEquipment::createDetachRequest() {
    return std::make_shared<DetachRequestMessage>(ueId_);
}

std::shared_ptr<Message> UserEquipment::createRegistrationRequest() {
    return std::make_shared<RegistrationRequestMessage>(ueId_, imsi_);
}

std::shared_ptr<Message> UserEquipment::createDataTransferMessage(SessionId sessionId, uint32_t dataSize) {
    return std::make_shared<DataTransferMessage>(ueId_, sessionId, dataSize);
}

void UserEquipment::printInfo() const {
    std::cout << "\n================== UE Information ==================\n";
    std::cout << "UE ID:              " << ueId_ << "\n";
    std::cout << "IMSI:               " << imsi_ << "\n";
    std::cout << "IMEI:               " << imei_ << "\n";
    std::cout << "Phone Number:       " << phoneNumber_ << "\n";
    std::cout << "State:              " << stateToString(state_) << "\n";
    std::cout << "Connected gNodeB:   " << connectedGnb_ << "\n";
    std::cout << "Current Session:    " << currentSessionId_ << "\n";
    std::cout << "Total UL Data:      " << totalUlData_ << " bytes\n";
    std::cout << "Total DL Data:      " << totalDlData_ << " bytes\n";
    std::cout << "===================================================\n\n";
}

std::string UserEquipment::getDetailedStatus() const {
    std::ostringstream oss;
    oss << "UE(" << ueId_ << ") - " << stateToString(state_)
        << " | gNB=" << connectedGnb_ 
        << " | UL=" << totalUlData_ << "B DL=" << totalDlData_ << "B";
    return oss.str();
}

std::string UserEquipment::stateToString(UeState state) const {
    switch (state) {
        case UeState::IDLE: return "IDLE";
        case UeState::SEARCHING: return "SEARCHING";
        case UeState::CONNECTED: return "CONNECTED";
        case UeState::REGISTERED: return "REGISTERED";
        case UeState::DISCONNECTED: return "DISCONNECTED";
        default: return "UNKNOWN";
    }
}
