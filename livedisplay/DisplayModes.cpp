/*
 * Copyright (C) 2019-2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "DisplayModesService"

#include <android-base/logging.h>
#include <fstream>

#include "DisplayModes.h"

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace implementation {

static const std::string kModeBasePath = "/sys/devices/virtual/graphics/fb0/";
static const std::string kDefaultPath = "/data/vendor/display/default_display_mode";

const std::map<int32_t, DisplayModes::ModeInfo> DisplayModes::kModeMap = {
    {0, {"Standard", "default"}},
    {1, {"Adaptive", "adaption_mode"}},
    {2, {"DCI-P3", "dci_p3"}},
    {3, {"OnePlus", "oneplus_mode"}},
    {4, {"sRGB", "srgb"}},
};

DisplayModes::DisplayModes()
    : mCurrentModeId(0), mDefaultModeId(0) {
    std::ifstream defaultFile(kDefaultPath);

    defaultFile >> mDefaultModeId;
    LOG(DEBUG) << "Default file read result " << mDefaultModeId << " fail " << defaultFile.fail();

    setDisplayMode(mDefaultModeId, false);
}

// Methods from ::vendor::lineage::livedisplay::V2_0::IDisplayModes follow.
Return<void> DisplayModes::getDisplayModes(getDisplayModes_cb resultCb) {
    std::vector<V2_0::DisplayMode> modes;

    for (const auto& entry : kModeMap) {
        modes.push_back({entry.first, entry.second.name});
    }
    resultCb(modes);
    return Void();
}

Return<void> DisplayModes::getCurrentDisplayMode(getCurrentDisplayMode_cb resultCb) {
    resultCb({mCurrentModeId, kModeMap.at(mCurrentModeId).name});
    return Void();
}

Return<void> DisplayModes::getDefaultDisplayMode(getDefaultDisplayMode_cb resultCb) {
    resultCb({mDefaultModeId, kModeMap.at(mDefaultModeId).name});
    return Void();
}

Return<bool> DisplayModes::setDisplayMode(int32_t modeID, bool makeDefault) {
    const auto iter = kModeMap.find(modeID);
    if (iter == kModeMap.end()) {
        return false;
    }
    for (const auto& entry : kModeMap) {
        if (entry.first == 0) {
            continue;
        }
        std::ofstream file(kModeBasePath + entry.second.node);
        file << 0;
        if (file.fail()) {
            LOG(ERROR) << "Failed to write to " << (kModeBasePath +  entry.second.node);
        }
    }
    std::ofstream file(kModeBasePath + iter->second.node);
    file << 1;
    if (file.fail()) {
        LOG(ERROR) << "Failed to write to " << (kModeBasePath +  iter->second.node);
    }
    mCurrentModeId = iter->first;
    if (makeDefault) {
        std::ofstream defaultFile(kDefaultPath);
        defaultFile << iter->first;
        if (!defaultFile.fail()) {
            mDefaultModeId = iter->first;
        }
    }
    if (mOnDisplayModeSet) {
        mOnDisplayModeSet();
    }
    return true;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
