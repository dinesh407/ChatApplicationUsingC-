cmake_minimum_required(VERSION 3.10)
project(5GCoreSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -pthread")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Source files
set(COMMON_SOURCES
    common/Message.cpp
    common/NetworkFunction.cpp
)

set(UE_SOURCES
    ue/UserEquipment.cpp
)

set(RAN_SOURCES
    ran/GNodeB.cpp
)

set(NRF_SOURCES
    nrf/NRF.cpp
)

set(AMF_SOURCES
    amf/AMF.cpp
)

set(SMF_SOURCES
    smf/SMF.cpp
)

set(UPF_SOURCES
    upf/UPF.cpp
)

set(PCF_SOURCES
    pcf/PCF.cpp
)

set(UDR_SOURCES
    udr/UDR.cpp
)

set(UDM_SOURCES
    udm/UDM.cpp
)

set(MAIN_SOURCES
    main/main.cpp
)

set(TEST_SOURCES
    main/test_single_ue_pcap.cpp
)

# All sources
set(ALL_SOURCES
    ${COMMON_SOURCES}
    ${UE_SOURCES}
    ${RAN_SOURCES}
    ${NRF_SOURCES}
    ${AMF_SOURCES}
    ${SMF_SOURCES}
    ${UPF_SOURCES}
    ${PCF_SOURCES}
    ${UDR_SOURCES}
    ${UDM_SOURCES}
    ${MAIN_SOURCES}
)

# All test sources
set(TEST_ALL_SOURCES
    ${COMMON_SOURCES}
    ${UE_SOURCES}
    ${RAN_SOURCES}
    ${NRF_SOURCES}
    ${AMF_SOURCES}
    ${SMF_SOURCES}
    ${UPF_SOURCES}
    ${PCF_SOURCES}
    ${UDR_SOURCES}
    ${UDM_SOURCES}
    ${TEST_SOURCES}
)

# Create main executable
add_executable(5g_simulator ${ALL_SOURCES})

# Create test executable
add_executable(5g_test_single_ue ${TEST_ALL_SOURCES})

# Link libraries
target_link_libraries(5g_simulator PRIVATE pthread)
target_link_libraries(5g_test_single_ue PRIVATE pthread)

# Optional: Add install target
install(TARGETS 5g_simulator 5g_test_single_ue DESTINATION bin)
