/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#ifndef INCLUDED_OculusRiftManager_h_GUID_C573D70C_AA30_426B_BB75_8C30A96711A4
#define INCLUDED_OculusRiftManager_h_GUID_C573D70C_AA30_426B_BB75_8C30A96711A4

// Internal Includes
#include "OculusRift.h"
#include "contains.h"
#include "ovr_version.h"
#include "make_unique.h"
#include "GetLastError.h"

// Library/third-party includes
#include <OVR_CAPI.h>
#include <osvr/PluginKit/PluginKit.h>

// Standard includes
#include <iostream>
#include <memory>

class OculusRiftManager;
using OculusRiftManagerSharedPtr = std::shared_ptr<OculusRiftManager>;
using OculusRiftManagerPtr = std::unique_ptr<OculusRiftManager>;

class OculusRiftManager {
public:
    OculusRiftManager();
    ~OculusRiftManager();

    // Oculus Rift Manager is non-copyable
    OculusRiftManager(const OculusRiftManager&) = delete;
    OculusRiftManager& operator=(const OculusRiftManager&) = delete;

    bool initialize();
    void shutdown();

    OSVR_ReturnCode detect(OSVR_PluginRegContext ctx);

private:
    bool initialized_{false};
    std::unique_ptr<OculusRift> oculusRift_;
};

inline OculusRiftManager::OculusRiftManager()
{
    initialize();
}

inline OculusRiftManager::~OculusRiftManager()
{
    shutdown();
}

#if OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,7,0,0)
void ovr_log_callback(uintptr_t /*user_data*/, int level, const char* message)
{
    std::cerr << "[OVR " << level << "] " << message << std::endl;
}
#elif OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,5,0,0)
void ovr_log_callback(int level, const char* message)
{
    std::cerr << "[OVR " << level << "] " << message << std::endl;
}
#endif

inline bool OculusRiftManager::initialize()
{
    std::cout << "[OSVR Oculus Rift] Initializing Oculus API..." << std::endl;


#if OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,7,0,0)
    ovrInitParams params = {
        ovrInit_Debug | ovrInit_RequestVersion, // Flags
        OVR_MINOR_VERSION,      // RequestedMinorVersion
        ovr_log_callback,       // LogCallback
        NULL,                   // UserData for LogCallback
        0                       // ConnectionTimeoutSeconds
    };
#elif OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,5,0,0)
    ovrInitParams params = {
        ovrInit_Debug | ovrInit_RequestVersion, // Flags
        OVR_MINOR_VERSION,      // RequestedMinorVersion
        ovr_log_callback,       // LogCallback
        0                       // ConnectionTimeoutSeconds
    };
#endif

#if OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,6,0,0)
    ovrResult result = ovr_Initialize(&params);
    initialized_ = OVR_SUCCESS(result);
#elif OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,5,0,0)
    ovrBool result = ovr_Initialize(&params);
    initialized_ = static_cast<bool>(result);
#elif OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,4,4,0)
    ovrBool result = ovr_Initialize();
    initialized_ = static_cast<bool>(result);
#else
#error "Unsupported version of Oculus VR SDK. Please upgrade."
#endif

    if (initialized_) {
        std::cout << "[OSVR Oculus Rift] Oculus Rift initialized." << std::endl;
    } else {
        std::cerr << "[OSVR Oculus Rift] Error initializing Oculus Rift system: " << getLastErrorMessage() << "." << std::endl;
    }

    return initialized_;
}

inline void OculusRiftManager::shutdown()
{
    std::cout << "[OSVR Oculus Rift] Shutting down Oculus API..." << std::endl;
    ovr_Shutdown();
}

inline OSVR_ReturnCode OculusRiftManager::detect(OSVR_PluginRegContext ctx)
{
    std::cout << "[OSVR Oculus Rift] Detecting Oculus Rifts..." << std::endl;
    if (!initialized_) {
        // Initialize the Oculus API and get a count of connected HMDs.
        std::cout << "[OSVR Oculus Rift] OVR system not initialized. Initializing..." << std::endl;
        initialize();
    }

    // Detect attached HMDs.
#if OSVR_OVR_VERSION_GREATER_OR_EQUAL(0,7,0,0)
    ovrHmdDesc hmd_description = ovr_GetHmdDesc(nullptr);
    const int num_hmds_detected = (hmd_description.Type == ovrHmd_None ? 0 : 1);
#else
    const int num_hmds_detected = ovrHmd_Detect();
#endif
    std::cout << "[OSVR Oculus Rift] Detected " << num_hmds_detected << (num_hmds_detected != 1 ? " HMDs." : " HMD.") << std::endl;

    // If not HMDs were detected and we still have a valid handle to one, we
    // should release that handle as the HMD has been unplugged.
    if (num_hmds_detected == 0 && oculusRift_)
        oculusRift_.reset();

    if (num_hmds_detected > 0) {
        if (!oculusRift_) {
            // If we detect an HMD and don't already have one created, we should
            // create it.
            const int index = 0;
            oculusRift_ = std::make_unique<OculusRift>(ctx, index);
        } else {
            // If we detect an HMD and we already have on created, check to see
            // if they're the same. If they are, do nothing. If they're
            // different, delete the old one and add the new one.
            // TODO
        }
    }

    return OSVR_RETURN_SUCCESS;
}

#endif // INCLUDED_OculusRiftManager_h_GUID_C573D70C_AA30_426B_BB75_8C30A96711A4

