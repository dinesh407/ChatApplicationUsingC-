#ifndef USER_EQUIPMENT_HPP
#define USER_EQUIPMENT_HPP

#include "../common/Types.hpp"
#include "../common/Logger.hpp"
#include "../common/Message.hpp"
#include <string>
#include <memory>
#include <chrono>
#include <map>

class UserEquipment {
public:
    UserEquipment(UeId ueId, Imsi imsi, Imei imei, const std::string& phoneNumber);
    ~UserEquipment() = default;

    // Getters
    UeId getUeId() const { return ueId_; }
    Imsi getImsi() const { return imsi_; }
    Imei getImei() const { return imei_; }
    std::string getPhoneNumber() const { return phoneNumber_; }
    UeState getState() const { return state_; }
    GnbId getConnectedGnb() const { return connectedGnb_; }
    SessionId getCurrentSessionId() const { return currentSessionId_; }

    // State management
    void setState(UeState newState);
    void attachToGnb(GnbId gnbId);
    void detachFromGnb();
    void registerAtCore();
    void deregister();

    // Session management
    void createSession(SessionId sessionId);
    void activateSession(SessionId sessionId);
    void deactivateSession(SessionId sessionId);
    void terminateSession(SessionId sessionId);

    // Traffic handling
    void sendData(SessionId sessionId, uint32_t bytes);
    void receiveData(SessionId sessionId, uint32_t bytes);

    // Message handling
    std::shared_ptr<Message> createAttachRequest();
    std::shared_ptr<Message> createDetachRequest();
    std::shared_ptr<Message> createRegistrationRequest();
    std::shared_ptr<Message> createDataTransferMessage(SessionId sessionId, uint32_t dataSize);

    // Statistics
    void printInfo() const;
    std::string getDetailedStatus() const;
    uint64_t getTotalUlData() const { return totalUlData_; }
    uint64_t getTotalDlData() const { return totalDlData_; }

private:
    UeId ueId_;
    Imsi imsi_;
    Imei imei_;
    std::string phoneNumber_;
    UeState state_;
    GnbId connectedGnb_;
    SessionId currentSessionId_;

    // Traffic statistics
    uint64_t totalUlData_;
    uint64_t totalDlData_;
    std::map<SessionId, uint64_t> sessionUlData_;
    std::map<SessionId, uint64_t> sessionDlData_;

    // Timestamps
    std::chrono::system_clock::time_point attachTime_;
    std::chrono::system_clock::time_point registrationTime_;

    Logger& logger_ = Logger::getInstance();

    std::string stateToString(UeState state) const;
};

#endif // USER_EQUIPMENT_HPP
