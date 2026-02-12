#ifndef PCF_HPP
#define PCF_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>
#include <set>

class PCF : public NetworkFunction {
public:
    explicit PCF();
    ~PCF() override = default;

    struct PolicyRule {
        std::string ruleName;
        UeId ueId;
        SessionId sessionId;
        uint32_t maxBitrate;
        uint32_t priorityLevel;
        bool isActive;
    };

    // Policy Management
    std::string createPolicy(UeId ueId, SessionId sessionId, uint32_t bitrate, uint32_t priority);
    bool updatePolicy(const std::string& policyId, uint32_t newBitrate);
    bool removePolicy(const std::string& policyId);
    PolicyRule* getPolicy(const std::string& policyId);

    // Charging Management
    void recordChargingEvent(UeId ueId, SessionId sessionId, uint64_t bytes);
    uint64_t getTotalCharge(UeId ueId) const;

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printActivePolicies() const;
    std::string getPCFStatus() const;
    uint32_t getActivePolicyCount() const { return policies_.size(); }

    void start() override;
    void stop() override;

private:
    std::map<std::string, PolicyRule> policies_;
    std::map<UeId, uint64_t> chargeRecords_;
    std::set<std::string> activePolicies_;

    std::string generatePolicyId();
    void logPolicyCreation(const std::string& policyId, UeId ueId);
    void logPolicyRemoval(const std::string& policyId);
};

#endif // PCF_HPP
