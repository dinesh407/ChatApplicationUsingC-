#ifndef PCAP_WRITER_HPP
#define PCAP_WRITER_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <cstring>

// PCAP Global Header Structure
struct PcapGlobalHeader {
    uint32_t magic_number;   // 0xa1b2c3d4
    uint16_t version_major;  // 2
    uint16_t version_minor;  // 4
    int32_t thiszone;        // 0
    uint32_t sigfigs;        // 0
    uint32_t snaplen;        // 65535
    uint32_t network;        // 1 (Ethernet)
};

// PCAP Packet Header Structure
struct PcapPacketHeader {
    uint32_t ts_sec;         // Timestamp seconds
    uint32_t ts_usec;        // Timestamp microseconds
    uint32_t incl_len;       // Included length
    uint32_t orig_len;       // Original length
};

class PcapWriter {
public:
    explicit PcapWriter(const std::string& filename) 
        : filename_(filename), file_(nullptr) {
        openFile();
    }

    ~PcapWriter() {
        closeFile();
    }

    void capturePacket(const std::string& source_ip, const std::string& dest_ip,
                      uint16_t source_port, uint16_t dest_port,
                      const std::string& protocol, const std::string& message_data) {
        if (!file_) return;

        // Create a simplified packet with headers
        std::vector<uint8_t> packet = createPacket(source_ip, dest_ip, source_port, 
                                                   dest_port, protocol, message_data);

        writePacket(packet);
    }

    void captureMessage(uint32_t source_id, uint32_t dest_id, uint16_t message_type,
                       const std::string& message_data) {
        if (!file_) return;

        // Create simplified packet representation
        std::vector<uint8_t> packet = createMessagePacket(source_id, dest_id, 
                                                          message_type, message_data);
        writePacket(packet);
    }

private:
    std::string filename_;
    std::FILE* file_;

    void openFile() {
        file_ = std::fopen(filename_.c_str(), "wb");
        if (!file_) {
            throw std::runtime_error("Failed to open PCAP file: " + filename_);
        }

        // Write global header
        PcapGlobalHeader global_header{
            0xa1b2c3d4,  // magic number
            2,           // version major
            4,           // version minor
            0,           // thiszone
            0,           // sigfigs
            65535,       // snaplen
            1            // network (Ethernet)
        };

        std::fwrite(&global_header, sizeof(global_header), 1, file_);
    }

    void closeFile() {
        if (file_) {
            std::fclose(file_);
            file_ = nullptr;
        }
    }

    std::vector<uint8_t> createPacket(const std::string& source_ip, 
                                      const std::string& dest_ip,
                                      uint16_t source_port, uint16_t dest_port,
                                      const std::string& protocol,
                                      const std::string& message_data) {
        std::vector<uint8_t> packet;

        // Ethernet Header (14 bytes)
        uint8_t dest_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        uint8_t src_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
        
        packet.insert(packet.end(), dest_mac, dest_mac + 6);
        packet.insert(packet.end(), src_mac, src_mac + 6);
        
        uint16_t eth_type = 0x0800;  // IPv4
        packet.push_back((eth_type >> 8) & 0xFF);
        packet.push_back(eth_type & 0xFF);

        // IP Header (20 bytes)
        packet.push_back(0x45);  // Version 4, IHL 5
        packet.push_back(0x00);  // DSCP, ECN
        uint16_t total_len = 20 + 8 + message_data.size();
        packet.push_back((total_len >> 8) & 0xFF);
        packet.push_back(total_len & 0xFF);

        // IP ID
        static uint16_t ip_id = 0;
        packet.push_back((ip_id >> 8) & 0xFF);
        packet.push_back(ip_id++ & 0xFF);

        packet.push_back(0x40);  // Flags (DF)
        packet.push_back(0x00);  // Fragment offset

        packet.push_back(0x40);  // TTL
        packet.push_back(0x11);  // Protocol (UDP)

        // Checksum (0x00, 0x00 for simplification)
        packet.push_back(0x00);
        packet.push_back(0x00);

        // Add simplified IP addresses (192.168.x.x format)
        uint8_t src_addr[] = {192, 168, 1, generateIpLastOctet(source_ip)};
        uint8_t dst_addr[] = {192, 168, 1, generateIpLastOctet(dest_ip)};
        
        packet.insert(packet.end(), src_addr, src_addr + 4);
        packet.insert(packet.end(), dst_addr, dst_addr + 4);

        // UDP Header (8 bytes)
        packet.push_back((source_port >> 8) & 0xFF);
        packet.push_back(source_port & 0xFF);
        packet.push_back((dest_port >> 8) & 0xFF);
        packet.push_back(dest_port & 0xFF);

        uint16_t udp_len = 8 + message_data.size();
        packet.push_back((udp_len >> 8) & 0xFF);
        packet.push_back(udp_len & 0xFF);

        // UDP Checksum (0x00, 0x00)
        packet.push_back(0x00);
        packet.push_back(0x00);

        // Payload
        packet.insert(packet.end(), message_data.begin(), message_data.end());

        return packet;
    }

    std::vector<uint8_t> createMessagePacket(uint32_t source_id, uint32_t dest_id,
                                            uint16_t message_type,
                                            const std::string& message_data) {
        // Create a custom header for 5G messages
        std::stringstream ss;
        ss << "SRC:" << std::hex << source_id 
           << "|DST:" << std::hex << dest_id
           << "|TYPE:" << std::hex << message_type
           << "|DATA:" << message_data;

        std::string payload = ss.str();
        return createPacket("NF-" + std::to_string(source_id),
                          "NF-" + std::to_string(dest_id),
                          5000 + (source_id % 1000),
                          5000 + (dest_id % 1000),
                          "5G-NF", payload);
    }

    uint8_t generateIpLastOctet(const std::string& id_str) {
        uint32_t hash = 0;
        for (char c : id_str) {
            hash = hash * 31 + c;
        }
        return (hash % 255) + 1;
    }

    void writePacket(const std::vector<uint8_t>& packet) {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration - seconds);

        PcapPacketHeader packet_header{
            static_cast<uint32_t>(seconds.count()),
            static_cast<uint32_t>(microseconds.count() % 1000000),
            static_cast<uint32_t>(packet.size()),
            static_cast<uint32_t>(packet.size())
        };

        std::fwrite(&packet_header, sizeof(packet_header), 1, file_);
        std::fwrite(packet.data(), packet.size(), 1, file_);
        std::fflush(file_);
    }
};

#endif // PCAP_WRITER_HPP
