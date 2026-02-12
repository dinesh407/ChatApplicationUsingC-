#ifndef GNODE_B_HPP
#define GNODE_B_HPP

#include "../common/Types.hpp"
#include "../common/Logger.hpp"
#include "../common/Message.hpp"
#include <string>
#include <memory>
#include <map>
#include <vector>

class GNodeB {
public:
    GNodeB(GnbId gnbId, const std::string& location);
    ~GNodeB() = default;

    // Getters
    GnbId getGnbId() const { return gnbId_; }
    std::string getLocation() const { return location_; }
    GnbState getState() const { return state_; }
    uint32_t getConnectedUeCount() const { return connectedUes_.size(); }
    uint32_t getCellCount() const { return cells_.size(); }

    // Cell management
    void addCell(uint32_t cellId, uint32_t pci, uint32_t frequency);
    CellInfo* getCell(uint32_t cellId);
    std::vector<CellInfo>& getAllCells() { return cells_; }

    // UE connection management
    void connectUe(UeId ueId);
    void disconnectUe(UeId ueId);
    bool isUeConnected(UeId ueId) const;
    uint32_t getConnectedUeCount(uint32_t cellId) const;

    // State management
    void setState(GnbState newState);

    // Traffic handling
    void updateTraffic(uint32_t ulBytes, uint32_t dlBytes);
    uint64_t getTotalUlTraffic() const { return totalUlTraffic_; }
    uint64_t getTotalDlTraffic() const { return totalDlTraffic_; }

    // Transmission Power and Coverage
    float getMeanRsrp(uint32_t cellId) const;
    float getMeanRsrq(uint32_t cellId) const;

    // Statistics and Information
    void printInfo() const;
    std::string getDetailedStatus() const;
    std::string getStatistics() const;

private:
    GnbId gnbId_;
    std::string location_;
    GnbState state_;

    std::vector<CellInfo> cells_;
    std::map<UeId, uint32_t> connectedUes_;  // UE ID -> Cell ID mapping

    uint64_t totalUlTraffic_;
    uint64_t totalDlTraffic_;

    Logger& logger_ = Logger::getInstance();

    std::string stateToString(GnbState state) const;
};

#endif // GNODE_B_HPP
