# 5G Core Network Function Simulator

A comprehensive C++ implementation of a 5G Core Network simulator with complete 3GPP-aligned network functions and realistic UE/RAN simulation.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        5G CORE NETWORK                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│  │   NRF    │  │   AMF    │  │   SMF    │  │   UPF    │         │
│  │(Service  │  │(Mobility)│  │(Session) │  │(Packets) │         │
│  │Discovery)│  │Management│  │Management│  │Forwarding│         │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘         │
│                                                                  │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                       │
│  │   PCF    │  │   UDR    │  │   UDM    │                       │
│  │(Policy)  │  │(Data)    │  │(Auth &   │                       │
│  │Control   │  │Repository│  │Subscript)│                       │
│  └──────────┘  └──────────┘  └──────────┘                       │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
          ↑                              ↑
          │                              │
    ┌─────┴──────────────┐      ┌────────┴──────────┐
    │    RAN/gNodeB      │      │   User Equipment  │
    │  (Base Stations)   │      │   (UE/Devices)    │
    └────────────────────┘      └───────────────────┘
```

## Network Functions

### 1. NRF (Network Repository Function)
- **Purpose**: Service discovery and NF instance registration
- **Key Features**:
  - NFInstance registration and deregistration
  - NFService discovery by type
  - Service availability management
  - Centralized NF directory

### 2. AMF (Access and Mobility Management Function)
- **Purpose**: UE registration, authentication, and mobility management
- **Key Features**:
  - UE registration management
  - Authentication and authorization
  - Mobility support (attach/detach/handover)
  - Registration context management
  - IMSI/IMEI validation

### 3. SMF (Session Management Function)
- **Purpose**: PDU session lifecycle management
- **Key Features**:
  - PDU session creation, activation, modification
  - Session termination
  - DNN (Data Network Name) support
  - IPv4/IPv6 address allocation
  - Session state management

### 4. UPF (User Plane Function)
- **Purpose**: Packet forwarding and QoS enforcement
- **Key Features**:
  - Uplink/downlink packet forwarding
  - Session attachment/detachment
  - QoS management and enforcement
  - Traffic metric collection
  - Session-level traffic accounting

### 5. PCF (Policy Control Function)
- **Purpose**: Policy and charging management
- **Key Features**:
  - Policy rule creation and management
  - QoS provisioning
  - Charging event recording
  - Priority level assignment
  - Policy enforcement

### 6. UDR (Unified Data Repository)
- **Purpose**: Centralized subscription data storage
- **Key Features**:
  - Subscription data storage and retrieval
  - Profile data management
  - Access control information
  - IMSI-indexed storage
  - Data persistence interface

### 7. UDM (Unified Data Management)
- **Purpose**: Authentication and subscription management
- **Key Features**:
  - Authentication challenge generation
  - Authentication response verification
  - Subscription information management
  - Public key distribution
  - Auth context lifecycle management

## RAN Components

### GNodeB (5G Base Station)
- **Features**:
  - Multiple cell support (up to 3 cells per gNodeB)
  - Physical Cell ID (PCI) management
  - RSRP/RSRQ simulation
  - UE connection tracking
  - Traffic statistics

## UE Components

### UserEquipment
- **Features**:
  - IMSI/IMEI management
  - State machine (IDLE, SEARCHING, CONNECTED, REGISTERED, DISCONNECTED)
  - Session lifecycle management
  - Traffic generation and reporting
  - gNodeB attachment/detachment

## Key Data Structures

### Message Types
- UE_ATTACH_REQUEST
- UE_ATTACH_ACCEPT
- REGISTRATION_REQUEST
- REGISTRATION_ACCEPT
- AUTHENTICATION_REQUEST
- PDU_SESSION_ESTABLISHMENT_REQUEST
- DATA_TRANSFER
- HEARTBEAT
- And more...

### Service Profile
```cpp
struct ServiceProfile {
    NFType nfType;
    std::string nfInstanceId;
    std::string nfName;
    std::vector<std::string> ipv4Addresses;
    uint16_t port;
    bool isAvailable;
};
```

### PDU Session Context
```cpp
struct PduSessionContext {
    SessionId pduSessionId;
    UeId ueId;
    SessionState state;
    Snssai snssai;
    std::string dnn;
    std::string ipv4Address;
    std::string ipv6Address;
    uint64_t ulTraffic;
    uint64_t dlTraffic;
};
```

## Building the Simulator

### Prerequisites
- C++ 17 compiler (g++, clang)
- CMake 3.10+
- pthread library

### Build Steps

```bash
cd /workspaces/ChatApplicationUsingC-/5gCORE
mkdir build
cd build
cmake ..
make
```

### Run the Simulator

```bash
./5g_simulator
```

## Simulation Scenarios

The simulator includes realistic simulation scenarios:

1. **UE Attachment**: UEs attach to gNodeBs and register with AMF
2. **PDU Session Establishment**: UEs establish PDU sessions through SMF
3. **Data Transfer**: Simulated uplink/downlink packet transfer
4. **Policy Enforcement**: PCF manages policies and charging
5. **Mobility**: Handovers between gNodeBs (supported)

## Logging System

The simulator includes a comprehensive logging system with color-coded output:

- **DEBUG** (Cyan): Detailed diagnostic information
- **INFO** (Green): General information messages
- **WARNING** (Yellow): Warning messages
- **ERROR** (Red): Error messages
- **CRITICAL** (Magenta): Critical error messages

## Performance Monitoring

The simulator provides real-time monitoring of:
- Active UE connections
- PDU session count
- Traffic statistics (UL/DL)
- Policy enforcement
- Charging records
- Authentication status

## Project Structure

```
5gCORE/
├── CMakeLists.txt              # Build configuration
├── common/                      # Shared utilities
│   ├── Types.hpp               # Type definitions
│   ├── Logger.hpp              # Logging system
│   ├── Message.hpp             # Message classes
│   ├── NetworkFunction.hpp     # Base NF class
│   ├── Message.cpp
│   └── NetworkFunction.cpp
├── ue/                          # User Equipment
│   ├── UserEquipment.hpp
│   └── UserEquipment.cpp
├── ran/                         # Radio Access Network
│   ├── GNodeB.hpp
│   └── GNodeB.cpp
├── nrf/                         # Network Repository Function
│   ├── NRF.hpp
│   └── NRF.cpp
├── amf/                         # Access and Mobility Management
│   ├── AMF.hpp
│   └── AMF.cpp
├── smf/                         # Session Management Function
│   ├── SMF.hpp
│   └── SMF.cpp
├── upf/                         # User Plane Function
│   ├── UPF.hpp
│   └── UPF.cpp
├── pcf/                         # Policy Control Function
│   ├── PCF.hpp
│   └── PCF.cpp
├── udr/                         # Unified Data Repository
│   ├── UDR.hpp
│   └── UDR.cpp
├── udm/                         # Unified Data Management
│   ├── UDM.hpp
│   └── UDM.cpp
└── main/                        # Main simulator
    └── main.cpp
```

## Class Hierarchy

```
NetworkFunction (Abstract Base)
    ├── NRF
    ├── AMF
    ├── SMF
    ├── UPF
    ├── PCF
    ├── UDR
    └── UDM

Message (Abstract Base)
    ├── AttachRequestMessage
    ├── DetachRequestMessage
    ├── AuthenticationRequestMessage
    ├── RegistrationRequestMessage
    ├── PduSessionEstablishmentRequestMessage
    └── DataTransferMessage
```

## Configuration Parameters

Key parameters that can be tuned:

- `MAX_UES`: Maximum number of UEs (default: 10000)
- `MAX_GNBS`: Maximum number of gNodeBs (default: 100)
- `MAX_SESSIONS`: Maximum PDU sessions (default: 50000)
- `MAX_CONNECTIONS_PER_GNB`: UEs per gNodeB (default: 1000)

## Features

✅ Complete 5G Core network simulation
✅ Full UE lifecycle management
✅ Realistic gNodeB simulation
✅ Service discovery (NRF)
✅ Authentication and authorization
✅ PDU session management
✅ QoS and policy enforcement
✅ Charging and billing
✅ Comprehensive logging
✅ Real-time statistics
✅ Extensible architecture
✅ C++ best practices (OOP, RAII, smart pointers)

## Future Enhancements

- Network slicing support (NSSAI)
- Advanced QoS handling
- Load balancing algorithms
- Multi-threaded message processing
- Network topology visualization
- Persistent data storage
- Performance profiling
- Unit tests and benchmarks

## License

This project is provided as-is for educational and research purposes.

## References

- 3GPP TS 23.501: System Architecture
- 3GPP TS 23.502: Procedures for 5GC
- 3GPP TS 38.401: E-UTRA and NR Multi-connectivity
