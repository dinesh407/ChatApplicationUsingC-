#ifndef NRF_HPP
#define NRF_HPP

#include "../common/NetworkFunction.hpp"
#include "../common/Types.hpp"
#include <map>
#include <vector>

class NRF : public NetworkFunction {
public:
    explicit NRF();
    ~NRF() override = default;

    // NFInstance Management
    void registerNFInstance(const ServiceProfile& profile);
    void deregisterNFInstance(const std::string& nfInstanceId);
    ServiceProfile* getNFInstance(const std::string& nfInstanceId);
    
    // NFService Discovery
    std::vector<ServiceProfile> discoverNFService(NFType nfType);
    ServiceProfile* discoverSpecificNF(NFType nfType, const std::string& preferredId = "");

    // NFService Availability
    void updateNFInstanceAvailability(const std::string& nfInstanceId, bool available);
    std::vector<ServiceProfile> getAvailableNFServices(NFType nfType);

    // Message handling
    void handleMessage(std::shared_ptr<Message> message) override;

    // Statistics
    void printNFDirectory() const;
    std::string getServiceDiscoveryStatus() const;
    uint32_t getNFInstanceCount(NFType nfType) const;

    void start() override;
    void stop() override;

private:
    std::map<std::string, ServiceProfile> nfServiceDirectory_;  // instanceId -> ServiceProfile
    std::map<NFType, std::vector<std::string>> nfTypeIndex_;    // NFType -> list of instanceIds

    ServiceProfile* findNFInstanceByType(NFType nfType);
    void logServiceRegistration(const ServiceProfile& profile);
    void logServiceDiscovery(NFType nfType, bool found);
};

#endif // NRF_HPP
