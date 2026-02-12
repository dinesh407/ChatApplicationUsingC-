#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <string>
#include <memory>
#include <map>
#include <vector>

// Basic Types
typedef uint32_t UeId;
typedef uint32_t GnbId;
typedef uint32_t SessionId;
typedef uint64_t Imsi;
typedef uint64_t Imei;
typedef uint32_t Snssai;  // Single Network Slice Selection Assistance Info

// State Enumerations
enum class UeState {
    IDLE,
    SEARCHING,
    CONNECTED,
    REGISTERED,
    DISCONNECTED
};

enum class GnbState {
    IDLE,
    ACTIVE,
    MAINTENANCE,
    UNAVAILABLE
};

enum class SessionState {
    IDLE,
    ACTIVATING,
    ACTIVE,
    MODIFYING,
    DEACTIVATING,
    TERMINATED
};

enum class MessageType {
    UE_ATTACH_REQUEST,
    UE_ATTACH_ACCEPT,
    UE_DETACH_REQUEST,
    UE_DETACH_ACCEPT,
    AUTHENTICATION_REQUEST,
    AUTHENTICATION_RESPONSE,
    SECURITY_MODE_COMMAND,
    SECURITY_MODE_COMPLETE,
    REGISTRATION_REQUEST,
    REGISTRATION_ACCEPT,
    SERVICE_REQUEST,
    SERVICE_ACCEPT,
    PDU_SESSION_ESTABLISHMENT_REQUEST,
    PDU_SESSION_ESTABLISHMENT_ACCEPT,
    PDU_SESSION_RELEASE_REQUEST,
    PDU_SESSION_RELEASE_COMPLETE,
    DATA_TRANSFER,
    HEARTBEAT,
    ERROR
};

// Network Function Types
enum class NFType {
    NRF,  // Network Repository Function
    AMF,  // Access and Mobility Management Function
    SMF,  // Session Management Function
    UPF,  // User Plane Function
    PCF,  // Policy Control Function
    UDR,  // Unified Data Repository
    UDM,  // Unified Data Management
    UE,   // User Equipment
    RAN   // Radio Access Network
};

// Protocol-related structures
struct ServiceProfile {
    NFType nfType;
    std::string nfInstanceId;
    std::string nfName;
    std::vector<std::string> ipv4Addresses;
    uint16_t port;
    bool isAvailable;
};

struct SubscriptionData {
    Imsi imsi;
    std::string msisdn;
    bool accessRestrictionData;
    std::vector<Snssai> allowedSnssais;
    std::map<std::string, std::string> additionalData;
};

struct S1apMessage {
    MessageType type;
    UeId ueId;
    uint32_t payloadSize;
    void* payload;
};

struct NgapMessage {
    MessageType type;
    UeId ueId;
    uint32_t payloadSize;
    void* payload;
};

// Common utility structures
struct CellInfo {
    uint32_t cellId;
    uint32_t pci;          // Physical Cell ID
    uint32_t frequency;    // Frequency in MHz
    float rsrp;            // Reference Signal Received Power
    float rsrq;            // Reference Signal Received Quality
};

struct PduSessionContext {
    SessionId pduSessionId;
    UeId ueId;
    SessionState state;
    Snssai snssai;
    std::string dnn;  // Data Network Name (e.g., "internet")
    std::string ipv4Address;
    std::string ipv6Address;
    uint64_t ulTraffic;
    uint64_t dlTraffic;
};

// Helper constants
constexpr uint16_t DEFAULT_SCTP_PORT = 132;
constexpr uint16_t DEFAULT_HTTP2_PORT = 8080;
constexpr uint32_t MAX_UES = 10000;
constexpr uint32_t MAX_GNBS = 100;
constexpr uint32_t MAX_SESSIONS = 50000;

#endif // TYPES_HPP
