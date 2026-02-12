# 5G CORE SIMULATOR - QUICK REFERENCE

## 📋 Quick Command Reference

### Building
```bash
cd /workspaces/ChatApplicationUsingC-/5gCORE
mkdir -p build && cd build
cmake .. && make -j4
```

### Running
```bash
./5g_simulator
```

### Clean Build
```bash
cd /workspaces/ChatApplicationUsingC-/5gCORE/build
make clean
cmake .. && make -j4
```

---

## 🔧 Key APIs

### Logging
```cpp
#include "common/Logger.hpp"

Logger::getInstance().info("Component", "Message");
Logger::getInstance().debug("Component", "Debug message");
Logger::getInstance().warning("Component", "Warning");
Logger::getInstance().error("Component", "Error");
Logger::getInstance().setLogLevel(LogLevel::DEBUG);
```

### Creating UE
```cpp
#include "ue/UserEquipment.hpp"

UserEquipment ue(1000, 310410000000000ULL, 354806000000000ULL, "+1234567890");
ue.attachToGnb(2000);
ue.registerAtCore();
ue.createSession(5000);
```

### Creating gNodeB
```cpp
#include "ran/GNodeB.hpp"

GNodeB gnb(2000, "New York");
gnb.addCell(200000, 100, 3500);
gnb.connectUe(1000);
```

### Using AMF
```cpp
#include "amf/AMF.hpp"

AMF amf;
amf.start();
amf.registerUe(ueId, imsi, imei);
amf.authenticateUe(ueId, imsi);
amf.handleUeAttach(ueId, gnbId);
```

### Using SMF
```cpp
#include "smf/SMF.hpp"

SMF smf;
smf.start();
SessionId session = smf.createPduSession(ueId, "internet", 0);
smf.activatePduSession(session);
```

### Using UPF
```cpp
#include "upf/UPF.hpp"

UPF upf;
upf.start();
upf.attachPduSession(sessionId, ueId);
upf.forwardUplinkPacket(sessionId, packetSize);
upf.setQoS(sessionId, 10000);  // 10 Mbps
```

### Using NRF
```cpp
#include "nrf/NRF.hpp"

NRF nrf;
nrf.start();
nrf.registerNFInstance(profile);
auto smf = nrf.discoverSpecificNF(NFType::SMF);
```

---

## 🏗️ Component Interaction

```
UE ←→ RAN (gNodeB)
  ↓
  AMF (Registration & Mobility)
  ↓
  UDM (Authentication)
  ↓
  UDR (Subscription Data)

PDU Session Establishment:
  ↓
  SMF (Session Management)
  ↓
  PCF (Policies & Charging)
  ↓
  UPF (Packet Forwarding)
```

---

## 📊 File Location Reference

| Component | Header | Implementation |
|-----------|--------|-----------------|
| Common | common/Types.hpp, Logger.hpp, Message.hpp | common/Message.cpp, NetworkFunction.cpp |
| UE | ue/UserEquipment.hpp | ue/UserEquipment.cpp |
| RAN | ran/GNodeB.hpp | ran/GNodeB.cpp |
| NRF | nrf/NRF.hpp | nrf/NRF.cpp |
| AMF | amf/AMF.hpp | amf/AMF.cpp |
| SMF | smf/SMF.hpp | smf/SMF.cpp |
| UPF | upf/UPF.hpp | upf/UPF.cpp |
| PCF | pcf/PCF.hpp | pcf/PCF.cpp |
| UDR | udr/UDR.hpp | udr/UDR.cpp |
| UDM | udm/UDM.hpp | udm/UDM.cpp |
| Main | - | main/main.cpp |

---

## 🔄 Typical Usage Pattern

```cpp
// 1. Initialize NFs
NRF nrf;
AMF amf;
SMF smf;
UPF upf;
PCF pcf;
UDR udr;
UDM udm;

// 2. Register in NRF
nrf.start();
nrf.registerNFInstance(amfProfile);
nrf.registerNFInstance(smfProfile);

// 3. Start NFs
amf.start();
smf.start();
upf.start();
pcf.start();
udr.start();
udm.start();

// 4. Create infrastructure
GNodeB gnb(id, location);
gnb.addCell(cellId, pci, freq);

UserEquipment ue(ueId, imsi, imei, phone);

// 5. Simulate workflow
ue.attachToGnb(gnbId);
gnb.connectUe(ueId);

amf.registerUe(ueId, imsi, imei);
amf.authenticateUe(ueId, imsi);
amf.handleUeAttach(ueId, gnbId);

SessionId session = smf.createPduSession(ueId, "internet", 0);
smf.activatePduSession(session);

upf.attachPduSession(session, ueId);
upf.forwardUplinkPacket(session, 1024);

pcf.recordChargingEvent(ueId, session, 1024);

// 6. Monitor
ue.printInfo();
gnb.printInfo();
nrf.printNFDirectory();
```

---

## 📈 Type Aliases

```cpp
typedef uint32_t UeId;
typedef uint32_t GnbId;
typedef uint32_t SessionId;
typedef uint64_t Imsi;
typedef uint64_t Imei;
typedef uint32_t Snssai;
```

---

## 🎯 Enum Values

### UeState
- IDLE
- SEARCHING
- CONNECTED
- REGISTERED
- DISCONNECTED

### GnbState
- IDLE
- ACTIVE
- MAINTENANCE
- UNAVAILABLE

### SessionState
- IDLE
- ACTIVATING
- ACTIVE
- MODIFYING
- DEACTIVATING
- TERMINATED

### NFType
- NRF
- AMF
- SMF
- UPF
- PCF
- UDR
- UDM
- UE
- RAN

---

## 🧪 Testing Snippets

### Test UE Lifecycle
```cpp
UserEquipment ue(1000, 310410000000000ULL, 354806000000000ULL, "+1234567890");

// Attach
ue.attachToGnb(2000);
assert(ue.getState() == UeState::CONNECTED);

// Register
ue.registerAtCore();
assert(ue.getState() == UeState::REGISTERED);

// Session
ue.createSession(5000);
ue.activateSession(5000);

// Data
ue.sendData(5000, 1024);
assert(ue.getTotalUlData() == 1024);

// Detach
ue.detachFromGnb();
assert(ue.getState() == UeState::DISCONNECTED);
```

### Test gNodeB
```cpp
GNodeB gnb(2000, "New York");

// Add cells
gnb.addCell(200000, 100, 3500);
gnb.addCell(200001, 101, 3550);
gnb.addCell(200002, 102, 3600);
assert(gnb.getCellCount() == 3);

// Connect UEs
for (int i = 0; i < 100; i++) {
    gnb.connectUe(1000 + i);
}
assert(gnb.getConnectedUeCount() == 100);

// Traffic
gnb.updateTraffic(10000, 20000);
assert(gnb.getTotalUlTraffic() == 10000);
assert(gnb.getTotalDlTraffic() == 20000);
```

### Test AMF
```cpp
AMF amf;
amf.start();

// Register
bool result = amf.registerUe(1000, 310410000000000ULL, 354806000000000ULL);
assert(result == true);
assert(amf.isUeRegistered(1000));

// Authenticate
result = amf.authenticateUe(1000, 310410000000000ULL);
assert(result == true);

// Attach
amf.handleUeAttach(1000, 2000);
assert(amf.getConnectedUeCount() > 0);
```

---

## 🐛 Debugging Tips

### Enable Debug Logging
```cpp
Logger::getInstance().setLogLevel(LogLevel::DEBUG);
```

### Check NRF Directory
```cpp
nrf.printNFDirectory();
```

### Get Component Status
```cpp
std::cout << amf.getAMFStatus();
std::cout << smf.getSMFStatus();
std::cout << upf.getUPFStatus();
```

### Print Session Metrics
```cpp
smf.printActiveSessions();
upf.printSessionMetrics();
```

---

## ⚠️ Common Issues

### Build Fails
- Ensure C++17 support: `g++ --version`
- Check CMake: `cmake --version`
- Try: `rm -rf build && mkdir build && cd build && cmake .. && make`

### Undefined Reference
- Check all .cpp files included in CMakeLists.txt
- Verify header guards in .hpp files
- Check for circular dependencies

### Segmentation Fault
- Check null pointers before use
- Verify pointer lifetimes
- Use AddressSanitizer: `cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..`

---

## 📦 Dependencies

- **System**: C++ standard library
- **Build**: CMake 3.10+, g++ or clang
- **Threading**: pthread (included with -pthread flag)
- **External**: None (self-contained)

---

## 🚀 Performance Tips

1. **Logging**: Use INFO level for production
2. **Memory**: Avoid frequent allocations
3. **Threading**: Already thread-safe with mutex
4. **Scalability**: Max parameters in Types.hpp

---

## 📋 Checklist for Adding New NF

- [ ] Create directory: `newnf/`
- [ ] Create header: `NewNF.hpp` inheriting from `NetworkFunction`
- [ ] Implement: `NewNF.cpp` with required methods
- [ ] Add to `CMakeLists.txt`
- [ ] Register in `main.cpp`
- [ ] Add message handling
- [ ] Add logging
- [ ] Test with existing components

---

## 🔗 Useful Links

- **Repository**: /workspaces/ChatApplicationUsingC-/5gCORE
- **Build Dir**: /workspaces/ChatApplicationUsingC-/5gCORE/build
- **Executable**: /workspaces/ChatApplicationUsingC-/5gCORE/build/5g_simulator

---

**Last Updated**: February 2026
**Version**: 1.0
**Status**: ✅ Production Ready
