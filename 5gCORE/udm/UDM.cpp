#include "UDM.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

UDM::UDM() : NetworkFunction(NFType::UDM, "UDM") {
    logger_.info(name_, "UDM initialized");
}

std::string UDM::generateAuthenticationChallenge(Imsi imsi) {
    std::string challenge = generateChallenge(imsi);

    AuthContext context;
    context.imsi = imsi;
    context.challenge = challenge;
    context.isAuthenticated = false;
    context.creationTime = std::chrono::system_clock::now();

    authContexts_[imsi] = context;

    logger_.info(name_, "Authentication challenge generated for IMSI: " + 
                       std::to_string(imsi));

    return challenge;
}

bool UDM::verifyAuthenticationResponse(Imsi imsi, const std::string& response) {
    auto it = authContexts_.find(imsi);
    if (it == authContexts_.end()) {
        logger_.error(name_, "No auth context found for IMSI: " + std::to_string(imsi));
        return false;
    }

    // Simple validation: response should be hash of challenge
    if (response.length() < 10) {
        logAuthenticationAttempt(imsi, false);
        return false;
    }

    it->second.isAuthenticated = true;
    logAuthenticationAttempt(imsi, true);

    return true;
}

bool UDM::getSubscriptionInfo(Imsi imsi, SubscriptionData* subData) {
    auto it = subscriptionCache_.find(imsi);
    if (it != subscriptionCache_.end()) {
        if (subData) {
            *subData = it->second;
        }
        logger_.debug(name_, "Subscription info retrieved for IMSI: " + 
                            std::to_string(imsi));
        return true;
    }

    logger_.warning(name_, "Subscription info not found for IMSI: " + 
                          std::to_string(imsi));
    return false;
}

bool UDM::updateSubscription(Imsi imsi, const SubscriptionData& subData) {
    subscriptionCache_[imsi] = subData;
    logger_.info(name_, "Subscription updated for IMSI: " + std::to_string(imsi));
    return true;
}

std::string UDM::getPublicKey(Imsi imsi) {
    auto it = publicKeyStore_.find(imsi);
    if (it != publicKeyStore_.end()) {
        return it->second;
    }

    // Generate default public key
    std::string pubKey = "PK_" + std::to_string(imsi);
    publicKeyStore_[imsi] = pubKey;

    logger_.debug(name_, "Public key generated for IMSI: " + std::to_string(imsi));

    return pubKey;
}

UDM::AuthContext* UDM::createAuthContext(Imsi imsi) {
    std::string challenge = generateAuthenticationChallenge(imsi);
    auto it = authContexts_.find(imsi);
    if (it != authContexts_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool UDM::destroyAuthContext(Imsi imsi) {
    auto it = authContexts_.find(imsi);
    if (it == authContexts_.end()) {
        return false;
    }

    authContexts_.erase(it);
    logger_.debug(name_, "Auth context destroyed for IMSI: " + std::to_string(imsi));

    return true;
}

void UDM::handleMessage(std::shared_ptr<Message> message) {
    if (!message) return;

    logger_.debug(name_, "Handling message: " + message->toString());

    switch (message->getType()) {
        case MessageType::AUTHENTICATION_REQUEST: {
            auto authMsg = std::dynamic_pointer_cast<AuthenticationRequestMessage>(message);
            if (authMsg) {
                logger_.info(name_, "Authentication challenge request received");
            }
            break;
        }
        case MessageType::REGISTRATION_REQUEST: {
            auto regMsg = std::dynamic_pointer_cast<RegistrationRequestMessage>(message);
            if (regMsg) {
                verifyAuthenticationResponse(regMsg->getImsi(), "dummy_response");
            }
            break;
        }
        default:
            logger_.warning(name_, "Unknown message type");
            break;
    }
}

void UDM::printAuthenticationStatus() const {
    std::cout << "\n================== UDM Authentication Status ==================\n";
    std::cout << "Active Auth Contexts: " << authContexts_.size() << "\n";
    std::cout << "Cached Subscriptions: " << subscriptionCache_.size() << "\n";
    std::cout << "Public Keys Stored: " << publicKeyStore_.size() << "\n\n";

    if (!authContexts_.empty()) {
        std::cout << "Active Authentication Contexts:\n";
        for (const auto& pair : authContexts_) {
            const auto& context = pair.second;
            std::cout << "  IMSI " << context.imsi 
                      << " | Challenge: " << context.challenge 
                      << " | Authenticated: " << (context.isAuthenticated ? "Yes" : "No") 
                      << "\n";
        }
    }
    std::cout << "=================================================================\n\n";
}

std::string UDM::getUDMStatus() const {
    std::ostringstream oss;
    oss << "UDM Status:\n"
        << "  Active Auth Contexts: " << authContexts_.size() << "\n"
        << "  Cached Subscriptions: " << subscriptionCache_.size() << "\n"
        << "  Public Keys: " << publicKeyStore_.size() << "\n";
    return oss.str();
}

std::string UDM::generateChallenge(Imsi imsi) {
    std::ostringstream oss;
    oss << "CHALLENGE_" << std::hex << imsi << "_" << std::time(nullptr);
    return oss.str();
}

void UDM::logAuthenticationAttempt(Imsi imsi, bool success) {
    if (success) {
        logger_.info(name_, "Authentication Successful | IMSI=" + std::to_string(imsi));
    } else {
        logger_.warning(name_, "Authentication Failed | IMSI=" + std::to_string(imsi));
    }
}

void UDM::start() {
    NetworkFunction::start();
    logger_.info(name_, "UDM started and ready for authentication");
}

void UDM::stop() {
    NetworkFunction::stop();
    authContexts_.clear();
    subscriptionCache_.clear();
    publicKeyStore_.clear();
    logger_.info(name_, "UDM stopped");
}
