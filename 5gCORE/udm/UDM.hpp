#ifndef UDM_HPP
#define UDM_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>
#include <memory>

class UDM : public NetworkFunction {
public:
    explicit UDM();
    ~UDM() override = default;

    // Authentication Management
    std::string generateAuthenticationChallenge(Imsi imsi);
    bool verifyAuthenticationResponse(Imsi imsi, const std::string& response);

    // Subscription Management
    bool getSubscriptionInfo(Imsi imsi, SubscriptionData* subData);
    bool updateSubscription(Imsi imsi, const SubscriptionData& subData);

    // Home Network Public Key Distribution (HNPK)
    std::string getPublicKey(Imsi imsi);

    // UE Authentication Context
    struct AuthContext {
        Imsi imsi;
        std::string challenge;
        bool isAuthenticated;
        std::chrono::system_clock::time_point creationTime;
    };

    AuthContext* createAuthContext(Imsi imsi);
    bool destroyAuthContext(Imsi imsi);

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printAuthenticationStatus() const;
    std::string getUDMStatus() const;
    uint32_t getActiveAuthContextCount() const { return authContexts_.size(); }

    void start() override;
    void stop() override;

private:
    std::map<Imsi, AuthContext> authContexts_;
    std::map<Imsi, SubscriptionData> subscriptionCache_;
    std::map<Imsi, std::string> publicKeyStore_;

    std::string generateChallenge(Imsi imsi);
    void logAuthenticationAttempt(Imsi imsi, bool success);
};

#endif // UDM_HPP
