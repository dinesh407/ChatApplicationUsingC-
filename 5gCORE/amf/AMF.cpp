#include "AMF.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>

AMF::AMF() : NetworkFunction(NFType::AMF, "AMF") {
    logger_.info(name_, "AMF initialized");
}

bool AMF::registerUe(UeId ueId, Imsi imsi, Imei imei) {
    if (registeredUes_.find(ueId) != registeredUes_.end()) {
        logger_.warning(name_, "UE already registered: " + std::to_string(ueId));
        return false;
    }

    if (!validateImsi(imsi)) {
        logger_.warning(name_, "Invalid IMSI for UE: " + std::to_string(ueId));
        return false;
    }

    if (!validateImei(imei)) {
        logger_.warning(name_, "Invalid IMEI for UE: " + std::to_string(ueId));
        return false;
    }

    UeRegistration registration;
    registration.ueId = ueId;
    registration.imsi = imsi;
    registration.imei = imei;
    registration.isAuthenticated = false;
    registration.isAuthorized = false;
    registration.connectedGnb = 0;
    registration.registrationTime = std::chrono::system_clock::now();

    registeredUes_[ueId] = registration;

    logUeRegistration(ueId, imsi);
    createRegistrationContext(ueId);

    return true;
}

bool AMF::deregisterUe(UeId ueId) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end()) {
        logger_.warning(name_, "UE not registered: " + std::to_string(ueId));
        return false;
    }

    registeredUes_.erase(it);
    connectedUes_.erase(ueId);
    ueContextMap_.erase(ueId);

    logUeDeregistration(ueId);

    return true;
}

bool AMF::isUeRegistered(UeId ueId) const {
    return registeredUes_.find(ueId) != registeredUes_.end();
}

bool AMF::authenticateUe(UeId ueId, Imsi imsi) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end()) {
        logger_.warning(name_, "Cannot authenticate: UE not found - " + std::to_string(ueId));
        return false;
    }

    if (it->second.imsi != imsi) {
        logger_.error(name_, "Authentication failed: IMSI mismatch for UE " + 
                            std::to_string(ueId));
        return false;
    }

    it->second.isAuthenticated = true;
    logger_.info(name_, "UE authenticated: " + std::to_string(ueId));

    return true;
}

bool AMF::authorizeUe(UeId ueId) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end() || !it->second.isAuthenticated) {
        logger_.warning(name_, "Cannot authorize: UE not authenticated - " + 
                               std::to_string(ueId));
        return false;
    }

    it->second.isAuthorized = true;
    logger_.info(name_, "UE authorized: " + std::to_string(ueId));

    return true;
}

void AMF::handleUeAttach(UeId ueId, GnbId gnbId) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end()) {
        logger_.error(name_, "UE attach failed: UE not registered - " + 
                            std::to_string(ueId));
        return;
    }

    it->second.connectedGnb = gnbId;
    connectedUes_.insert(ueId);

    logger_.info(name_, "UE attached: " + std::to_string(ueId) + 
                       " to gNodeB " + std::to_string(gnbId));
}

void AMF::handleUeDetach(UeId ueId) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end()) {
        logger_.warning(name_, "UE detach: UE not found - " + std::to_string(ueId));
        return;
    }

    GnbId previousGnb = it->second.connectedGnb;
    it->second.connectedGnb = 0;
    connectedUes_.erase(ueId);

    logger_.info(name_, "UE detached: " + std::to_string(ueId) + 
                       " from gNodeB " + std::to_string(previousGnb));
}

void AMF::handleHandover(UeId ueId, GnbId sourceGnb, GnbId targetGnb) {
    auto it = registeredUes_.find(ueId);
    if (it == registeredUes_.end()) {
        logger_.warning(name_, "Handover failed: UE not found - " + std::to_string(ueId));
        return;
    }

    if (it->second.connectedGnb != sourceGnb) {
        logger_.error(name_, "Handover failed: Source gNodeB mismatch for UE " + 
                            std::to_string(ueId));
        return;
    }

    it->second.connectedGnb = targetGnb;

    logger_.info(name_, "Handover complete: UE " + std::to_string(ueId) + 
                       " from gNodeB " + std::to_string(sourceGnb) + 
                       " to gNodeB " + std::to_string(targetGnb));
}

void AMF::createRegistrationContext(UeId ueId) {
    ueContextMap_[ueId] = "REG_CONTEXT_" + std::to_string(ueId);
    logger_.debug(name_, "Registration context created for UE " + std::to_string(ueId));
}

void AMF::createAmfContext(UeId ueId) {
    logger_.debug(name_, "AMF context created for UE " + std::to_string(ueId));
}

void AMF::deleteRegistrationContext(UeId ueId) {
    ueContextMap_.erase(ueId);
    logger_.debug(name_, "Registration context deleted for UE " + std::to_string(ueId));
}

void AMF::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::UE_ATTACH_REQUEST: {
            auto attachMsg = std::dynamic_pointer_cast<AttachRequestMessage>(message);
            if (attachMsg) {
                registerUe(message->getSourceId(), attachMsg->getImsi(), attachMsg->getImei());
            }
            break;
        }
        case MessageType::REGISTRATION_REQUEST: {
            auto regMsg = std::dynamic_pointer_cast<RegistrationRequestMessage>(message);
            if (regMsg) {
                authenticateUe(message->getSourceId(), regMsg->getImsi());
                authorizeUe(message->getSourceId());
            }
            break;
        }
        case MessageType::UE_DETACH_REQUEST:
            deregisterUe(message->getSourceId());
            break;
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void AMF::printRegisteredUes() const {
    std::cout << "\n================== AMF Registered UEs ==================\n";
    std::cout << "Total Registered UEs: " << registeredUes_.size() << "\n";
    std::cout << "Connected UEs: " << connectedUes_.size() << "\n\n";

    for (const auto& pair : registeredUes_) {
        const auto& reg = pair.second;
        std::cout << "UE ID: " << reg.ueId << " | IMSI: " << reg.imsi 
                  << " | Authenticated: " << (reg.isAuthenticated ? "Yes" : "No")
                  << " | Connected gNB: " << reg.connectedGnb << "\n";
    }
    std::cout << "========================================================\n\n";
}

std::string AMF::getAMFStatus() const {
    std::ostringstream oss;
    oss << "AMF Status:\n"
        << "  Registered UEs: " << registeredUes_.size() << "\n"
        << "  Connected UEs: " << connectedUes_.size() << "\n"
        << "  Registration Contexts: " << ueContextMap_.size() << "\n";
    return oss.str();
}

bool AMF::validateImsi(Imsi imsi) {
    // Basic IMSI validation: IMSI should be between certain ranges
    return imsi > 0 && imsi < 1000000000000000ULL;
}

bool AMF::validateImei(Imei imei) {
    // Basic IMEI validation: IMEI should be between certain ranges
    return imei > 0 && imei < 1000000000000000ULL;
}

void AMF::logUeRegistration(UeId ueId, Imsi imsi) {
    logger_.info(name_, "UE Registration | ID=" + std::to_string(ueId) + 
                       " | IMSI=" + std::to_string(imsi));
}

void AMF::logUeDeregistration(UeId ueId) {
    logger_.info(name_, "UE Deregistration | ID=" + std::to_string(ueId));
}

void AMF::start() {
    NetworkFunction::start();
    logger_.info(name_, "AMF started and ready for registration");
}

void AMF::stop() {
    NetworkFunction::stop();
    registeredUes_.clear();
    connectedUes_.clear();
    logger_.info(name_, "AMF stopped");
}
