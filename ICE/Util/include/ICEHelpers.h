#pragma once

/**
 * @file ICEHelpers.h
 * @brief Convenience header that includes all ICE helper utilities
 * 
 * This header provides simplified APIs for common ICE operations,
 * reducing verbosity and improving developer experience.
 * 
 * Usage:
 *   #include <ICEHelpers.h>
 * 
 *   // Simplified entity access
 *   EntityHelper player(playerId, registry);
 *   player.transform()->setPosition({0, 1, 0});
 * 
 *   // Simplified engine access
 *   auto registry = EngineHelper::getRegistry(engine);
 *   auto assetBank = EngineHelper::getAssetBank(engine);
 */

#include "EntityHelper.h"
#include "EngineHelper.h"
