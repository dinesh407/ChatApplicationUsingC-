#ifndef AMF_HPP
#define AMF_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>
#include <set>

class AMF : public NetworkFunction {
public:
    explicit AMF();
    ~AMF() override = default;

    // UE Registration Management
    bool registerUe(UeId ueId, Imsi imsi, Imei imei);
    bool deregisterUe(UeId ueId);
    bool isUeRegistered(UeId ueId) const;

    // Authentication
    bool authenticateUe(UeId ueId, Imsi imsi);
    bool authorizeUe(UeId ueId);

    // Mobility Management
    void handleUeAttach(UeId ueId, GnbId gnbId);
    void handleUeDetach(UeId ueId);
    void handleHandover(UeId ueId, GnbId sourceGnb, GnbId targetGnb);

    // Session Management
    void createRegistrationContext(UeId ueId);
    void createAmfContext(UeId ueId);
    void deleteRegistrationContext(UeId ueId);

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics and Information
    void printRegisteredUes() const;
    std::string getAMFStatus() const;
    uint32_t getRegisteredUeCount() const { return registeredUes_.size(); }
    uint32_t getConnectedUeCount() const { return connectedUes_.size(); }

    void start() override;
    void stop() override;

private:
    struct UeRegistration {
        UeId ueId;
        Imsi imsi;
        Imei imei;
        bool isAuthenticated;
        bool isAuthorized;
        GnbId connectedGnb;
        std::chrono::system_clock::time_point registrationTime;
    };

    std::map<UeId, UeRegistration> registeredUes_;
    std::set<UeId> connectedUes_;
    std::map<UeId, std::string> ueContextMap_;

    bool validateImsi(Imsi imsi);
    bool validateImei(Imei imei);
    void logUeRegistration(UeId ueId, Imsi imsi);
    void logUeDeregistration(UeId ueId);
};

#endif // AMF_HPP
