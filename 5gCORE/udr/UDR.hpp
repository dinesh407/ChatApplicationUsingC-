#ifndef UDR_HPP
#define UDR_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>

class UDR : public NetworkFunction {
public:
    explicit UDR();
    ~UDR() override = default;

    // Subscription Data Management
    bool storeSubscriptionData(Imsi imsi, const SubscriptionData& subData);
    SubscriptionData* getSubscriptionData(Imsi imsi);
    bool updateSubscriptionData(Imsi imsi, const SubscriptionData& subData);
    bool removeSubscriptionData(Imsi imsi);

    // Profile Data Storage
    void storeProfileData(UeId ueId, const std::map<std::string, std::string>& profileData);
    std::map<std::string, std::string>* getProfileData(UeId ueId);

    // Access Control Information
    void storeAccessInfo(Imsi imsi, const std::string& accessInfo);
    std::string getAccessInfo(Imsi imsi) const;

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printStoredData() const;
    std::string getUDRStatus() const;
    uint32_t getStoredSubscriptionCount() const { return subscriptionDatabase_.size(); }

    void start() override;
    void stop() override;

private:
    std::map<Imsi, SubscriptionData> subscriptionDatabase_;
    std::map<UeId, std::map<std::string, std::string>> profileDatabase_;
    std::map<Imsi, std::string> accessControlInfo_;

    void logDataStored(Imsi imsi);
    void logDataRetrieved(Imsi imsi);
};

#endif // UDR_HPP
