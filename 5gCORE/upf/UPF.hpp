#ifndef UPF_HPP
#define UPF_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>

class UPF : public NetworkFunction {
public:
    explicit UPF();
    ~UPF() override = default;

    // UPF Functionality
    void attachPduSession(SessionId sessionId, UeId ueId);
    void detachPduSession(SessionId sessionId);

    // Packet Forwarding
    void forwardUplinkPacket(SessionId sessionId, uint32_t packetSize);
    void forwardDownlinkPacket(SessionId sessionId, uint32_t packetSize);

    // QoS Management
    void setQoS(SessionId sessionId, uint32_t bitrate);
    uint32_t getQoS(SessionId sessionId) const;

    // Traffic Metrics
    uint64_t getTotalUplinkTraffic() const { return totalUplinkTraffic_; }
    uint64_t getTotalDownlinkTraffic() const { return totalDownlinkTraffic_; }
    uint64_t getSessionUplinkTraffic(SessionId sessionId) const;
    uint64_t getSessionDownlinkTraffic(SessionId sessionId) const;

    // Message Handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printSessionMetrics() const;
    std::string getUPFStatus() const;
    uint32_t getAttachedSessionCount() const { return attachedSessions_.size(); }

    void start() override;
    void stop() override;

private:
    struct SessionMetrics {
        SessionId sessionId;
        UeId ueId;
        uint64_t uplinkBytes;
        uint64_t downlinkBytes;
        uint32_t qosRate;  // in kbps
        bool isAttached;
    };

    std::map<SessionId, SessionMetrics> attachedSessions_;
    uint64_t totalUplinkTraffic_;
    uint64_t totalDownlinkTraffic_;

    void logPacketForwarding(SessionId sessionId, bool isUplink, uint32_t size);
};

#endif // UPF_HPP
