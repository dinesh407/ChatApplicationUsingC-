#include "NRF.hpp"
#include <iostream>
#include <algorithm>

NRF::NRF() : NetworkFunction(NFType::NRF, "NRF") {
    logger_.info(name_, "NRF initialized");
}

void NRF::registerNFInstance(const ServiceProfile& profile) {
    if (nfServiceDirectory_.find(profile.nfInstanceId) != nfServiceDirectory_.end()) {
        logger_.warning(name_, "NF Instance already registered: " + profile.nfInstanceId);
        return;
    }

    nfServiceDirectory_[profile.nfInstanceId] = profile;
    nfTypeIndex_[profile.nfType].push_back(profile.nfInstanceId);

    logServiceRegistration(profile);
}

void NRF::deregisterNFInstance(const std::string& nfInstanceId) {
    auto it = nfServiceDirectory_.find(nfInstanceId);
    if (it == nfServiceDirectory_.end()) {
        logger_.warning(name_, "NF Instance not found: " + nfInstanceId);
        return;
    }

    NFType nfType = it->second.nfType;
    nfServiceDirectory_.erase(it);

    auto& typeVector = nfTypeIndex_[nfType];
    typeVector.erase(std::remove(typeVector.begin(), typeVector.end(), nfInstanceId), 
                     typeVector.end());

    logger_.info(name_, "NF Instance deregistered: " + nfInstanceId);
}

ServiceProfile* NRF::getNFInstance(const std::string& nfInstanceId) {
    auto it = nfServiceDirectory_.find(nfInstanceId);
    if (it != nfServiceDirectory_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<ServiceProfile> NRF::discoverNFService(NFType nfType) {
    std::vector<ServiceProfile> results;
    
    auto it = nfTypeIndex_.find(nfType);
    if (it != nfTypeIndex_.end()) {
        for (const auto& instanceId : it->second) {
            auto profile = getNFInstance(instanceId);
            if (profile && profile->isAvailable) {
                results.push_back(*profile);
            }
        }
    }

    logServiceDiscovery(nfType, !results.empty());
    return results;
}

ServiceProfile* NRF::discoverSpecificNF(NFType nfType, const std::string& preferredId) {
    // If preferred ID is specified, try to find it
    if (!preferredId.empty()) {
        auto profile = getNFInstance(preferredId);
        if (profile && profile->nfType == nfType && profile->isAvailable) {
            return profile;
        }
    }

    // Otherwise, find any available NF of the specified type
    auto it = nfTypeIndex_.find(nfType);
    if (it != nfTypeIndex_.end()) {
        for (const auto& instanceId : it->second) {
            auto profile = getNFInstance(instanceId);
            if (profile && profile->isAvailable) {
                logger_.debug(name_, "Service discovered: " + 
                             std::to_string(static_cast<int>(nfType)) + 
                             " -> " + profile->nfInstanceId);
                return profile;
            }
        }
    }

    logger_.warning(name_, "No available NF service found for type: " + 
                          std::to_string(static_cast<int>(nfType)));
    return nullptr;
}

void NRF::updateNFInstanceAvailability(const std::string& nfInstanceId, bool available) {
    auto it = nfServiceDirectory_.find(nfInstanceId);
    if (it != nfServiceDirectory_.end()) {
        it->second.isAvailable = available;
        logger_.info(name_, "NF Instance " + nfInstanceId + " availability: " + 
                          (available ? "AVAILABLE" : "UNAVAILABLE"));
    }
}

std::vector<ServiceProfile> NRF::getAvailableNFServices(NFType nfType) {
    std::vector<ServiceProfile> results;
    
    auto it = nfTypeIndex_.find(nfType);
    if (it != nfTypeIndex_.end()) {
        for (const auto& instanceId : it->second) {
            auto profile = getNFInstance(instanceId);
            if (profile && profile->isAvailable) {
                results.push_back(*profile);
            }
        }
    }

    return results;
}

void NRF::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::UE_ATTACH_REQUEST:
            logger_.info(name_, "Processing attachment request from UE");
            break;
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void NRF::printNFDirectory() const {
    std::cout << "\n======================= NRF Service Directory =======================\n";
    std::cout << "Total Registered NF Instances: " << nfServiceDirectory_.size() << "\n\n";

    for (const auto& pair : nfServiceDirectory_) {
        const auto& profile = pair.second;
        std::cout << "Instance ID: " << profile.nfInstanceId << "\n";
        std::cout << "  Type:        " << static_cast<int>(profile.nfType) << "\n";
        std::cout << "  Name:        " << profile.nfName << "\n";
        std::cout << "  Port:        " << profile.port << "\n";
        std::cout << "  Available:   " << (profile.isAvailable ? "Yes" : "No") << "\n";
        if (!profile.ipv4Addresses.empty()) {
            std::cout << "  IPv4:        " << profile.ipv4Addresses[0] << "\n";
        }
        std::cout << "\n";
    }
    std::cout << "====================================================================\n\n";
}

std::string NRF::getServiceDiscoveryStatus() const {
    std::string status = "NRF Service Directory Status:\n";
    for (const auto& pair : nfTypeIndex_) {
        status += "  Type " + std::to_string(static_cast<int>(pair.first)) + ": " + 
                 std::to_string(pair.second.size()) + " instances\n";
    }
    return status;
}

uint32_t NRF::getNFInstanceCount(NFType nfType) const {
    auto it = nfTypeIndex_.find(nfType);
    if (it != nfTypeIndex_.end()) {
        return it->second.size();
    }
    return 0;
}

void NRF::logServiceRegistration(const ServiceProfile& profile) {
    logger_.info(name_, "NF Service Registered | Type=" + std::to_string(static_cast<int>(profile.nfType)) + 
                        " | ID=" + profile.nfInstanceId + 
                        " | Name=" + profile.nfName);
}

void NRF::logServiceDiscovery(NFType nfType, bool found) {
    if (found) {
        logger_.debug(name_, "NF Service discovered for Type=" + std::to_string(static_cast<int>(nfType)));
    } else {
        logger_.warning(name_, "No NF Service found for Type=" + std::to_string(static_cast<int>(nfType)));
    }
}

void NRF::start() {
    NetworkFunction::start();
    logger_.info(name_, "NRF started and ready for service discovery");
}

void NRF::stop() {
    NetworkFunction::stop();
    logger_.info(name_, "NRF stopped");
}
