#include "GNodeB.hpp"
#include <iostream>
#include <sstream>
#include <cmath>
#include <iomanip>

GNodeB::GNodeB(GnbId gnbId, const std::string& location)
    : gnbId_(gnbId), location_(location), state_(GnbState::ACTIVE),
      totalUlTraffic_(0), totalDlTraffic_(0) {
    logger_.info("RAN", "Creating gNodeB: ID=" + std::to_string(gnbId) + 
                        ", Location=" + location);
}

void GNodeB::addCell(uint32_t cellId, uint32_t pci, uint32_t frequency) {
    if (cells_.size() >= 3) {  // Max 3 cells per gNodeB in this simulation
        logger_.warning("RAN", "gNodeB " + std::to_string(gnbId_) + 
                               " already has maximum cells");
        return;
    }

    CellInfo cell;
    cell.cellId = cellId;
    cell.pci = pci;
    cell.frequency = frequency;
    cell.rsrp = -70.0f + (rand() % 30);
    cell.rsrq = -5.0f + (rand() % 15);

    cells_.push_back(cell);

    logger_.info("RAN", "gNodeB " + std::to_string(gnbId_) + 
                        " added cell: ID=" + std::to_string(cellId) +
                        ", PCI=" + std::to_string(pci) +
                        ", Freq=" + std::to_string(frequency) + "MHz");
}

CellInfo* GNodeB::getCell(uint32_t cellId) {
    for (auto& cell : cells_) {
        if (cell.cellId == cellId) {
            return &cell;
        }
    }
    return nullptr;
}

void GNodeB::connectUe(UeId ueId) {
    if (isUeConnected(ueId)) {
        logger_.warning("RAN", "UE " + std::to_string(ueId) + 
                               " already connected to gNodeB " + std::to_string(gnbId_));
        return;
    }

    if (connectedUes_.size() >= 1000) {  // Max 1000 UEs per gNodeB
        logger_.warning("RAN", "gNodeB " + std::to_string(gnbId_) + 
                               " at maximum capacity");
        return;
    }

    // Connect to first available cell
    uint32_t cellId = cells_.empty() ? 0 : cells_[0].cellId;
    connectedUes_[ueId] = cellId;

    logger_.info("RAN", "UE " + std::to_string(ueId) + " connected to gNodeB " + 
                        std::to_string(gnbId_) + " (Cell=" + std::to_string(cellId) + 
                        ", Total UEs=" + std::to_string(connectedUes_.size()) + ")");
}

void GNodeB::disconnectUe(UeId ueId) {
    auto it = connectedUes_.find(ueId);
    if (it != connectedUes_.end()) {
        connectedUes_.erase(it);
        logger_.info("RAN", "UE " + std::to_string(ueId) + 
                           " disconnected from gNodeB " + std::to_string(gnbId_));
    }
}

bool GNodeB::isUeConnected(UeId ueId) const {
    return connectedUes_.find(ueId) != connectedUes_.end();
}

uint32_t GNodeB::getConnectedUeCount(uint32_t cellId) const {
    uint32_t count = 0;
    for (const auto& pair : connectedUes_) {
        if (pair.second == cellId) {
            count++;
        }
    }
    return count;
}

void GNodeB::setState(GnbState newState) {
    if (state_ == newState) return;

    std::string oldState = stateToString(state_);
    std::string newStateStr = stateToString(newState);

    logger_.debug("RAN", "gNodeB " + std::to_string(gnbId_) + 
                         " state transition: " + oldState + " -> " + newStateStr);

    state_ = newState;
}

void GNodeB::updateTraffic(uint32_t ulBytes, uint32_t dlBytes) {
    totalUlTraffic_ += ulBytes;
    totalDlTraffic_ += dlBytes;
}

float GNodeB::getMeanRsrp(uint32_t cellId) const {
    for (const auto& cell : cells_) {
        if (cell.cellId == cellId) {
            return cell.rsrp;
        }
    }
    return 0.0f;
}

float GNodeB::getMeanRsrq(uint32_t cellId) const {
    for (const auto& cell : cells_) {
        if (cell.cellId == cellId) {
            return cell.rsrq;
        }
    }
    return 0.0f;
}

void GNodeB::printInfo() const {
    std::cout << "\n================== gNodeB Information ==================\n";
    std::cout << "gNodeB ID:          " << gnbId_ << "\n";
    std::cout << "Location:           " << location_ << "\n";
    std::cout << "State:              " << stateToString(state_) << "\n";
    std::cout << "Number of Cells:    " << cells_.size() << "\n";
    std::cout << "Connected UEs:      " << connectedUes_.size() << "\n";
    std::cout << "Total UL Traffic:   " << totalUlTraffic_ << " bytes\n";
    std::cout << "Total DL Traffic:   " << totalDlTraffic_ << " bytes\n";

    if (!cells_.empty()) {
        std::cout << "\nCell Information:\n";
        for (const auto& cell : cells_) {
            std::cout << "  Cell ID=" << cell.cellId
                      << " | PCI=" << cell.pci
                      << " | Freq=" << cell.frequency << "MHz"
                      << " | RSRP=" << std::fixed << std::setprecision(1) << cell.rsrp << "dBm"
                      << " | RSRQ=" << cell.rsrq << "dB\n";
        }
    }
    std::cout << "=======================================================\n\n";
}

std::string GNodeB::getDetailedStatus() const {
    std::ostringstream oss;
    oss << "gNB(" << gnbId_ << ") - " << stateToString(state_)
        << " | Location=" << location_
        << " | UEs=" << connectedUes_.size()
        << " | Cells=" << cells_.size();
    return oss.str();
}

std::string GNodeB::getStatistics() const {
    std::ostringstream oss;
    oss << "gNodeB " << gnbId_ << " Statistics:\n"
        << "  UL Traffic: " << totalUlTraffic_ << " bytes\n"
        << "  DL Traffic: " << totalDlTraffic_ << " bytes\n"
        << "  Total Traffic: " << (totalUlTraffic_ + totalDlTraffic_) << " bytes\n";
    return oss.str();
}

std::string GNodeB::stateToString(GnbState state) const {
    switch (state) {
        case GnbState::IDLE: return "IDLE";
        case GnbState::ACTIVE: return "ACTIVE";
        case GnbState::MAINTENANCE: return "MAINTENANCE";
        case GnbState::UNAVAILABLE: return "UNAVAILABLE";
        default: return "UNKNOWN";
    }
}
