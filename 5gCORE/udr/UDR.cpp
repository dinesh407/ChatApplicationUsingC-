#include "UDR.hpp"
#include <iostream>

UDR::UDR() : NetworkFunction(NFType::UDR, "UDR") {
    logger_.info(name_, "UDR initialized");
}

bool UDR::storeSubscriptionData(Imsi imsi, const SubscriptionData& subData) {
    if (subscriptionDatabase_.find(imsi) != subscriptionDatabase_.end()) {
        logger_.warning(name_, "Subscription data already exists for IMSI: " + 
                               std::to_string(imsi));
        return false;
    }

    subscriptionDatabase_[imsi] = subData;
    logDataStored(imsi);

    return true;
}

SubscriptionData* UDR::getSubscriptionData(Imsi imsi) {
    auto it = subscriptionDatabase_.find(imsi);
    if (it != subscriptionDatabase_.end()) {
        logDataRetrieved(imsi);
        return &it->second;
    }
    logger_.warning(name_, "Subscription data not found for IMSI: " + 
                          std::to_string(imsi));
    return nullptr;
}

bool UDR::updateSubscriptionData(Imsi imsi, const SubscriptionData& subData) {
    auto it = subscriptionDatabase_.find(imsi);
    if (it == subscriptionDatabase_.end()) {
        logger_.warning(name_, "Cannot update: Subscription data not found for IMSI: " + 
                               std::to_string(imsi));
        return false;
    }

    it->second = subData;
    logger_.info(name_, "Subscription data updated for IMSI: " + std::to_string(imsi));

    return true;
}

bool UDR::removeSubscriptionData(Imsi imsi) {
    auto it = subscriptionDatabase_.find(imsi);
    if (it == subscriptionDatabase_.end()) {
        logger_.warning(name_, "Cannot remove: Subscription data not found for IMSI: " + 
                               std::to_string(imsi));
        return false;
    }

    subscriptionDatabase_.erase(it);
    logger_.info(name_, "Subscription data removed for IMSI: " + std::to_string(imsi));

    return true;
}

void UDR::storeProfileData(UeId ueId, const std::map<std::string, std::string>& profileData) {
    profileDatabase_[ueId] = profileData;
    logger_.debug(name_, "Profile data stored for UE: " + std::to_string(ueId));
}

std::map<std::string, std::string>* UDR::getProfileData(UeId ueId) {
    auto it = profileDatabase_.find(ueId);
    if (it != profileDatabase_.end()) {
        return &it->second;
    }
    return nullptr;
}

void UDR::storeAccessInfo(Imsi imsi, const std::string& accessInfo) {
    accessControlInfo_[imsi] = accessInfo;
    logger_.debug(name_, "Access info stored for IMSI: " + std::to_string(imsi));
}

std::string UDR::getAccessInfo(Imsi imsi) const {
    auto it = accessControlInfo_.find(imsi);
    if (it != accessControlInfo_.end()) {
        return it->second;
    }
    return "";
}

void UDR::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::REGISTRATION_REQUEST: {
            auto regMsg = std::dynamic_pointer_cast<RegistrationRequestMessage>(message);
            if (regMsg) {
                getSubscriptionData(regMsg->getImsi());
            }
            break;
        }
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void UDR::printStoredData() const {
    std::cout << "\n================== UDR Stored Data ==================\n";
    std::cout << "Stored Subscriptions: " << subscriptionDatabase_.size() << "\n";
    std::cout << "Profile Records: " << profileDatabase_.size() << "\n";
    std::cout << "Access Control Records: " << accessControlInfo_.size() << "\n\n";

    if (!subscriptionDatabase_.empty()) {
        std::cout << "Subscription Data:\n";
        for (const auto& pair : subscriptionDatabase_) {
            const auto& subData = pair.second;
            std::cout << "  IMSI " << subData.imsi 
                      << " | MSISDN: " << subData.msisdn 
                      << " | Access Allowed: " << (subData.accessRestrictionData ? "No" : "Yes") 
                      << "\n";
        }
    }
    std::cout << "=====================================================\n\n";
}

std::string UDR::getUDRStatus() const {
    std::ostringstream oss;
    oss << "UDR Status:\n"
        << "  Stored Subscriptions: " << subscriptionDatabase_.size() << "\n"
        << "  Profile Records: " << profileDatabase_.size() << "\n"
        << "  Access Control Records: " << accessControlInfo_.size() << "\n";
    return oss.str();
}

void UDR::logDataStored(Imsi imsi) {
    logger_.info(name_, "Subscription Data Stored | IMSI=" + std::to_string(imsi));
}

void UDR::logDataRetrieved(Imsi imsi) {
    logger_.debug(name_, "Subscription Data Retrieved | IMSI=" + std::to_string(imsi));
}

void UDR::start() {
    NetworkFunction::start();
    logger_.info(name_, "UDR started and ready for data management");
}

void UDR::stop() {
    NetworkFunction::stop();
    subscriptionDatabase_.clear();
    profileDatabase_.clear();
    accessControlInfo_.clear();
    logger_.info(name_, "UDR stopped");
}
