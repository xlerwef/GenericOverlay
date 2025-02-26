// SteamOverlay.hpp
#pragma once
#include <memory>
#include <functional>
#include "OverlayConfig.hpp"
#include "ModuleSignature.hpp"
#include "include/MinHook.h"

namespace Overlay {
    class SteamOverlay {
    public:
        struct HookCallbacks {
            std::function<void(void*)> onPresent;
            std::function<void(void*)> onCreate;
        };

        static SteamOverlay& getInstance() {
            static SteamOverlay instance;
            return instance;
        }

        bool initialize(const std::string& processName, const HookCallbacks& callbacks) {
            if (MH_Initialize() != MH_OK) {
                return false;
            }

            auto& config = OverlayConfig::getInstance();

            // Try to load config, fall back to defaults if needed
            if (!loadHooks(processName)) {
                return false;
            }

            setupCallbacks(callbacks);
            return true;
        }

        void cleanup() {
            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();
        }

    private:
        SteamOverlay() = default;

        bool loadHooks(const std::string& processName) {
            auto& config = OverlayConfig::getInstance();

            // First try to load from config
            auto presentPattern = config.getPattern("present");
            auto createPattern = config.getPattern("create");

            // Fall back to defaults if not found
            if (!presentPattern) {
                presentPattern = SignaturePattern("present",
                    OverlayConfig::DEFAULT_PRESENT_PATTERN,
                    "gameoverlayrenderer64.dll");
            }

            if (!createPattern) {
                createPattern = SignaturePattern("create",
                    OverlayConfig::DEFAULT_CREATE_PATTERN,
                    "gameoverlayrenderer64.dll");
            }

            // Find and create hooks
            auto present = ModuleSignature(presentPattern->module, presentPattern->pattern).find();
            auto create = ModuleSignature(createPattern->module, createPattern->pattern).find();

            if (!present || !create) {
                return false;
            }

            m_presentHook = present;
            m_createHook = create;

            return true;
        }

        void setupCallbacks(const HookCallbacks& callbacks) {
            if (callbacks.onPresent) {
                MH_CreateHook(m_presentHook,
                    reinterpret_cast<void*>(callbacks.onPresent),
                    reinterpret_cast<void**>(&m_originalPresent));
                MH_EnableHook(m_presentHook);
            }

            if (callbacks.onCreate) {
                MH_CreateHook(m_createHook,
                    reinterpret_cast<void*>(callbacks.onCreate),
                    reinterpret_cast<void**>(&m_originalCreate));
                MH_EnableHook(m_createHook);
            }
        }

        void* m_presentHook = nullptr;
        void* m_createHook = nullptr;
        void* m_originalPresent = nullptr;
        void* m_originalCreate = nullptr;
    };
}