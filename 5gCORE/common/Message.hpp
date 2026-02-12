#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "Types.hpp"
#include <chrono>
#include <memory>

class Message {
public:
    Message(MessageType type, uint32_t sourceId, uint32_t destId)
        : type_(type), sourceId_(sourceId), destId_(destId),
          messageId_(++messageCounter_), 
          timestamp_(std::chrono::system_clock::now()) {}

    virtual ~Message() = default;

    MessageType getType() const { return type_; }
    uint32_t getSourceId() const { return sourceId_; }
    uint32_t getDestId() const { return destId_; }
    uint32_t getMessageId() const { return messageId_; }
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }

    virtual std::string toString() const = 0;

protected:
    MessageType type_;
    uint32_t sourceId_;
    uint32_t destId_;
    uint32_t messageId_;
    std::chrono::system_clock::time_point timestamp_;

private:
    static uint32_t messageCounter_;
};

// Specific Message Types
class AttachRequestMessage : public Message {
public:
    AttachRequestMessage(UeId ueId, uint64_t imsi, uint64_t imei)
        : Message(MessageType::UE_ATTACH_REQUEST, ueId, 0),
          imsi_(imsi), imei_(imei) {}

    uint64_t getImsi() const { return imsi_; }
    uint64_t getImei() const { return imei_; }

    std::string toString() const override {
        return "AttachRequest(UE=" + std::to_string(sourceId_) + 
               ", IMSI=" + std::to_string(imsi_) + ")";
    }

private:
    uint64_t imsi_;
    uint64_t imei_;
};

class DetachRequestMessage : public Message {
public:
    DetachRequestMessage(UeId ueId)
        : Message(MessageType::UE_DETACH_REQUEST, ueId, 0) {}

    std::string toString() const override {
        return "DetachRequest(UE=" + std::to_string(sourceId_) + ")";
    }
};

class AuthenticationRequestMessage : public Message {
public:
    AuthenticationRequestMessage(UeId ueId, const std::string& challenge)
        : Message(MessageType::AUTHENTICATION_REQUEST, ueId, 0),
          challenge_(challenge) {}

    std::string getChallenge() const { return challenge_; }

    std::string toString() const override {
        return "AuthenticationRequest(UE=" + std::to_string(sourceId_) + ")";
    }

private:
    std::string challenge_;
};

class RegistrationRequestMessage : public Message {
public:
    RegistrationRequestMessage(UeId ueId, uint64_t imsi)
        : Message(MessageType::REGISTRATION_REQUEST, ueId, 0),
          imsi_(imsi) {}

    uint64_t getImsi() const { return imsi_; }

    std::string toString() const override {
        return "RegistrationRequest(UE=" + std::to_string(sourceId_) + 
               ", IMSI=" + std::to_string(imsi_) + ")";
    }

private:
    uint64_t imsi_;
};

class PduSessionEstablishmentRequestMessage : public Message {
public:
    PduSessionEstablishmentRequestMessage(UeId ueId, SessionId sessionId, const std::string& dnn)
        : Message(MessageType::PDU_SESSION_ESTABLISHMENT_REQUEST, ueId, 0),
          sessionId_(sessionId), dnn_(dnn) {}

    SessionId getSessionId() const { return sessionId_; }
    std::string getDnn() const { return dnn_; }

    std::string toString() const override {
        return "PduSessionEstablishmentRequest(UE=" + std::to_string(sourceId_) + 
               ", Session=" + std::to_string(sessionId_) + ", DNN=" + dnn_ + ")";
    }

private:
    SessionId sessionId_;
    std::string dnn_;
};

class DataTransferMessage : public Message {
public:
    DataTransferMessage(UeId ueId, SessionId sessionId, uint32_t dataSize)
        : Message(MessageType::DATA_TRANSFER, ueId, 0),
          sessionId_(sessionId), dataSize_(dataSize) {}

    SessionId getSessionId() const { return sessionId_; }
    uint32_t getDataSize() const { return dataSize_; }

    std::string toString() const override {
        return "DataTransfer(UE=" + std::to_string(sourceId_) + 
               ", Session=" + std::to_string(sessionId_) + 
               ", Size=" + std::to_string(dataSize_) + "B)";
    }

private:
    SessionId sessionId_;
    uint32_t dataSize_;
};

#endif // MESSAGE_HPP
