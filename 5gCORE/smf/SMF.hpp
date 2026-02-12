#ifndef SMF_HPP
#define SMF_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>

class SMF : public NetworkFunction {
public:
    explicit SMF();
    ~SMF() override = default;

    // PDU Session Management
    SessionId createPduSession(UeId ueId, const std::string& dnn, Snssai snssai);
    bool activatePduSession(SessionId sessionId);
    bool modifyPduSession(SessionId sessionId, const std::string& newDnn);
    bool releasePduSession(SessionId sessionId);
    bool terminatePduSession(SessionId sessionId);

    // Session Queries
    PduSessionContext* getPduSessionContext(SessionId sessionId);
    bool isSessionActive(SessionId sessionId) const;
    std::vector<SessionId> getActiveSessions(UeId ueId) const;

    // Traffic Handling
    void recordUplink(SessionId sessionId, uint64_t bytes);
    void recordDownlink(SessionId sessionId, uint64_t bytes);

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printActiveSessions() const;
    std::string getSMFStatus() const;
    uint32_t getActiveSessionCount() const { return activeSessions_.size(); }

    void start() override;
    void stop() override;

private:
    std::map<SessionId, PduSessionContext> pduSessions_;
    std::map<UeId, std::vector<SessionId>> ueSessionMap_;
    std::map<SessionId, SessionState> activeSessions_;

    std::string generateIpv4Address();
    std::string generateIpv6Address();
    void logSessionCreation(SessionId sessionId, UeId ueId);
    void logSessionActivation(SessionId sessionId);
    void logSessionTermination(SessionId sessionId);
};

#endif // SMF_HPP
