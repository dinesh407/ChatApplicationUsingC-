#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

// Common headers
#include "common/Types.hpp"
#include "common/Logger.hpp"
#include "common/Message.hpp"

// Component headers
#include "ue/UserEquipment.hpp"
#include "ran/GNodeB.hpp"
#include "nrf/NRF.hpp"
#include "amf/AMF.hpp"
#include "smf/SMF.hpp"
#include "upf/UPF.hpp"
#include "pcf/PCF.hpp"
#include "udr/UDR.hpp"
#include "udm/UDM.hpp"

class FiveGSimulator {
public:
    FiveGSimulator() {
        logger_.info("SIMULATOR", "Initializing 5G Core Network Simulator");
    }

    void initialize() {
        // Initialize Core Network Functions
        nrf_ = std::make_shared<NRF>();
        amf_ = std::make_shared<AMF>();
        smf_ = std::make_shared<SMF>();
        upf_ = std::make_shared<UPF>();
        pcf_ = std::make_shared<PCF>();
        udr_ = std::make_shared<UDR>();
        udm_ = std::make_shared<UDM>();

        // Register NF instances in NRF
        registerNFServices();

        // Start all network functions
        startNetworkFunctions();

        logger_.info("SIMULATOR", "5G Core Network initialized successfully");
    }

    void registerNFServices() {
        // Register AMF
        ServiceProfile amfProfile;
        amfProfile.nfType = NFType::AMF;
        amfProfile.nfInstanceId = amf_->getInstanceId();
        amfProfile.nfName = "AMF-Instance-1";
        amfProfile.port = 38412;
        amfProfile.isAvailable = true;
        amfProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(amfProfile);

        // Register SMF
        ServiceProfile smfProfile;
        smfProfile.nfType = NFType::SMF;
        smfProfile.nfInstanceId = smf_->getInstanceId();
        smfProfile.nfName = "SMF-Instance-1";
        smfProfile.port = 8080;
        smfProfile.isAvailable = true;
        smfProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(smfProfile);

        // Register UPF
        ServiceProfile upfProfile;
        upfProfile.nfType = NFType::UPF;
        upfProfile.nfInstanceId = upf_->getInstanceId();
        upfProfile.nfName = "UPF-Instance-1";
        upfProfile.port = 2152;
        upfProfile.isAvailable = true;
        upfProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(upfProfile);

        // Register PCF
        ServiceProfile pcfProfile;
        pcfProfile.nfType = NFType::PCF;
        pcfProfile.nfInstanceId = pcf_->getInstanceId();
        pcfProfile.nfName = "PCF-Instance-1";
        pcfProfile.port = 8080;
        pcfProfile.isAvailable = true;
        pcfProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(pcfProfile);

        // Register UDR
        ServiceProfile udrProfile;
        udrProfile.nfType = NFType::UDR;
        udrProfile.nfInstanceId = udr_->getInstanceId();
        udrProfile.nfName = "UDR-Instance-1";
        udrProfile.port = 8080;
        udrProfile.isAvailable = true;
        udrProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(udrProfile);

        // Register UDM
        ServiceProfile udmProfile;
        udmProfile.nfType = NFType::UDM;
        udmProfile.nfInstanceId = udm_->getInstanceId();
        udmProfile.nfName = "UDM-Instance-1";
        udmProfile.port = 8080;
        udmProfile.isAvailable = true;
        udmProfile.ipv4Addresses.push_back("127.0.0.1");
        nrf_->registerNFInstance(udmProfile);
    }

    void startNetworkFunctions() {
        nrf_->start();
        amf_->start();
        smf_->start();
        upf_->start();
        pcf_->start();
        udr_->start();
        udm_->start();
    }

    void createUEs(uint32_t count) {
        for (uint32_t i = 0; i < count; ++i) {
            UeId ueId = 1000 + i;
            Imsi imsi = 310410000000000ULL + i;
            Imei imei = 354806000000000ULL + i;
            std::string phoneNumber = "+1234567890" + std::to_string(i);

            auto ue = std::make_unique<UserEquipment>(ueId, imsi, imei, phoneNumber);
            ues_.push_back(std::move(ue));

            logger_.info("SIMULATOR", "Created UE: ID=" + std::to_string(ueId) + 
                                     ", IMSI=" + std::to_string(imsi));
        }
    }

    void createGNodeBs(uint32_t count) {
        std::vector<std::string> locations = {"New York", "Los Angeles", "Chicago", "Houston", "Phoenix"};

        for (uint32_t i = 0; i < count; ++i) {
            GnbId gnbId = 2000 + i;
            std::string location = locations[i % locations.size()] + "_gNB_" + std::to_string(i);

            auto gnb = std::make_unique<GNodeB>(gnbId, location);

            // Add cells to each gNodeB
            for (uint32_t j = 0; j < 3; ++j) {
                gnb->addCell(gnbId * 100 + j, 100 + j, 3500 + j * 50);
            }

            gnbs_.push_back(std::move(gnb));

            logger_.info("SIMULATOR", "Created gNodeB: ID=" + std::to_string(gnbId) + 
                                     ", Location=" + location);
        }
    }

    void simulateUEAttachment() {
        logger_.info("SIMULATOR", "=== Simulating UE Attachment ===");

        for (size_t i = 0; i < ues_.size() && i < gnbs_.size(); ++i) {
            // Attach UE to gNodeB
            ues_[i]->attachToGnb(gnbs_[i % gnbs_.size()]->getGnbId());
            gnbs_[i % gnbs_.size()]->connectUe(ues_[i]->getUeId());

            // Register UE at AMF
            amf_->registerUe(ues_[i]->getUeId(), ues_[i]->getImsi(), ues_[i]->getImei());
            amf_->authenticateUe(ues_[i]->getUeId(), ues_[i]->getImsi());
            amf_->handleUeAttach(ues_[i]->getUeId(), gnbs_[i % gnbs_.size()]->getGnbId());

            // Store subscription data
            SubscriptionData subData;
            subData.imsi = ues_[i]->getImsi();
            subData.msisdn = ues_[i]->getPhoneNumber();
            subData.accessRestrictionData = false;
            udr_->storeSubscriptionData(ues_[i]->getImsi(), subData);

            // Set UE to registered state
            ues_[i]->registerAtCore();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void simulatePDUSessionEstablishment() {
        logger_.info("SIMULATOR", "=== Simulating PDU Session Establishment ===");

        for (size_t i = 0; i < ues_.size() && i <= 2; ++i) {  // Create sessions for first 3 UEs
            SessionId sessionId = smf_->createPduSession(ues_[i]->getUeId(), "internet", 0);
            smf_->activatePduSession(sessionId);

            // Create policy in PCF
            std::string policyId = pcf_->createPolicy(ues_[i]->getUeId(), sessionId, 10000, 9);

            // Attach session to UPF
            upf_->attachPduSession(sessionId, ues_[i]->getUeId());
            upf_->setQoS(sessionId, 10000);  // 10 Mbps

            ues_[i]->createSession(sessionId);
            ues_[i]->activateSession(sessionId);

            logger_.info("SIMULATOR", "PDU Session established for UE " + 
                                     std::to_string(ues_[i]->getUeId()));

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void simulateDataTransfer() {
        logger_.info("SIMULATOR", "=== Simulating Data Transfer ===");

        for (size_t i = 0; i < ues_.size() && i <= 2; ++i) {
            // Simulate uplink data transfer
            uint32_t dataSize = 1024 * (rand() % 100);  // 0-100 KB
            ues_[i]->sendData(i + 5000, dataSize);

            // Record in UPF
            upf_->forwardUplinkPacket(i + 5000, dataSize);

            // Record charging event in PCF
            pcf_->recordChargingEvent(ues_[i]->getUeId(), i + 5000, dataSize);

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    void printSimulatorStatus() {
        system("clear");
        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║            5G CORE NETWORK FUNCTION SIMULATOR                      ║\n";
        std::cout << "║  A comprehensive C++ implementation with UE and RAN simulation    ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

        std::cout << "\n┌─ NETWORK FUNCTIONS STATUS ─────────────────────────────────────────┐\n";
        std::cout << "│ NRF Status: " << (nrf_->getIsRunning() ? "✓ Running" : "✗ Stopped") << "\n";
        std::cout << "│ AMF Status: " << (amf_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Registered UEs: " << amf_->getRegisteredUeCount() << "\n";
        std::cout << "│ SMF Status: " << (smf_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Active Sessions: " << smf_->getActiveSessionCount() << "\n";
        std::cout << "│ UPF Status: " << (upf_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Attached Sessions: " << upf_->getAttachedSessionCount() << "\n";
        std::cout << "│ PCF Status: " << (pcf_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Active Policies: " << pcf_->getActivePolicyCount() << "\n";
        std::cout << "│ UDR Status: " << (udr_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Subscriptions: " << udr_->getStoredSubscriptionCount() << "\n";
        std::cout << "│ UDM Status: " << (udm_->getIsRunning() ? "✓ Running" : "✗ Stopped") 
                  << " | Auth Contexts: " << udm_->getActiveAuthContextCount() << "\n";
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";

        std::cout << "\n┌─ INFRASTRUCTURE STATUS ────────────────────────────────────────────┐\n";
        std::cout << "│ Total UEs Created: " << ues_.size() << "\n";
        std::cout << "│ Total gNodeBs: " << gnbs_.size() << "\n";

        if (!ues_.empty()) {
            std::cout << "\n│ UE Status:\n";
            for (size_t i = 0; i < std::min(size_t(5), ues_.size()); ++i) {
                std::cout << "│   " << ues_[i]->getDetailedStatus() << "\n";
            }
            if (ues_.size() > 5) {
                std::cout << "│   ... and " << (ues_.size() - 5) << " more UEs\n";
            }
        }

        if (!gnbs_.empty()) {
            std::cout << "\n│ gNodeB Status:\n";
            for (size_t i = 0; i < std::min(size_t(3), gnbs_.size()); ++i) {
                std::cout << "│   " << gnbs_[i]->getDetailedStatus() << "\n";
            }
        }
        std::cout << "└────────────────────────────────────────────────────────────────────┘\n";
    }

    void runDetailedDemo() {
        std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                   DETAILED SYSTEM INFORMATION                      ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

        std::cout << "\n=== NRF Service Directory ===\n";
        nrf_->printNFDirectory();

        std::cout << "\n=== AMF Registered UEs ===\n";
        amf_->printRegisteredUes();

        std::cout << "\n=== SMF Active Sessions ===\n";
        smf_->printActiveSessions();

        std::cout << "\n=== UPF Session Metrics ===\n";
        upf_->printSessionMetrics();

        std::cout << "\n=== PCF Active Policies ===\n";
        pcf_->printActivePolicies();

        std::cout << "\n=== UDR Stored Data ===\n";
        udr_->printStoredData();

        std::cout << "\n=== UDM Authentication Status ===\n";
        udm_->printAuthenticationStatus();

        std::cout << "\n=== Sample gNodeB Information ===\n";
        if (!gnbs_.empty()) {
            gnbs_[0]->printInfo();
        }

        std::cout << "\n=== Sample UE Information ===\n";
        if (!ues_.empty()) {
            ues_[0]->printInfo();
        }
    }

    void shutdown() {
        logger_.info("SIMULATOR", "Shutting down simulator...");

        nrf_->stop();
        amf_->stop();
        smf_->stop();
        upf_->stop();
        pcf_->stop();
        udr_->stop();
        udm_->stop();

        ues_.clear();
        gnbs_.clear();

        logger_.info("SIMULATOR", "Simulator shut down successfully");
    }

private:
    std::shared_ptr<NRF> nrf_;
    std::shared_ptr<AMF> amf_;
    std::shared_ptr<SMF> smf_;
    std::shared_ptr<UPF> upf_;
    std::shared_ptr<PCF> pcf_;
    std::shared_ptr<UDR> udr_;
    std::shared_ptr<UDM> udm_;

    std::vector<std::unique_ptr<UserEquipment>> ues_;
    std::vector<std::unique_ptr<GNodeB>> gnbs_;

    Logger& logger_ = Logger::getInstance();
};

int main() {
    Logger::getInstance().setLogLevel(LogLevel::INFO);

    FiveGSimulator simulator;
    simulator.initialize();

    // Create network infrastructure
    simulator.createGNodeBs(3);
    simulator.createUEs(5);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run simulation scenarios
    simulator.simulateUEAttachment();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    simulator.simulatePDUSessionEstablishment();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    simulator.simulateDataTransfer();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Display status
    simulator.printSimulatorStatus();

    std::cout << "\nPress Enter to view detailed system information...";
    std::cin.get();

    simulator.runDetailedDemo();

    std::cout << "\nPress Enter to shutdown simulator...";
    std::cin.get();

    simulator.shutdown();

    return 0;
}
