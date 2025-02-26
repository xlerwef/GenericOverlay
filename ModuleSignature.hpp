#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <string_view>

namespace Overlay {
    class ModuleSignature {
    public:
        ModuleSignature(std::string_view moduleName, std::string_view patternString)
            : m_moduleName(moduleName), m_patternString(patternString) {}

        void* find() const {
            HMODULE hModule = GetModuleHandleA(m_moduleName.data());
            if (!hModule) {
                std::cerr << "Error: Module not found: " << m_moduleName << std::endl;
                return nullptr;
            }

            std::vector<byte> pattern;
            std::vector<byte> mask;
            if (!parsePattern(pattern, mask)) {
                return nullptr;
            }

            size_t moduleSize = getModuleSize(hModule);
            if (moduleSize == 0) {
                return nullptr;
            }

            return scanPattern(hModule, moduleSize, pattern, mask);
        }

    private:
        bool parsePattern(std::vector<byte>& pattern, std::vector<byte>& mask) const {
            std::istringstream iss(m_patternString.data());
            std::string byteString;

            while (std::getline(iss, byteString, ' ')) {
                if (byteString == "?") {
                    pattern.push_back(0);
                    mask.push_back(0);
                }
                else {
                    try {
                        unsigned int byteValue;
                        std::stringstream ss;
                        ss << std::hex << byteString;
                        ss >> byteValue;
                        pattern.push_back(static_cast<byte>(byteValue));
                        mask.push_back(0xFF);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error parsing pattern: " << e.what() << std::endl;
                        return false;
                    }
                }
            }
            return !pattern.empty();
        }

        size_t getModuleSize(HMODULE hModule) const {
            size_t moduleSize = 0;
            void* currentAddress = static_cast<void*>(hModule);
            const size_t MAX_MODULE_SIZE = 0x1000000; // 16MB safety limit

            while (true) {
                MEMORY_BASIC_INFORMATION mbi;
                if (VirtualQuery(currentAddress, &mbi, sizeof(mbi)) == 0) {
                    break;
                }

                moduleSize += mbi.RegionSize;
                currentAddress = static_cast<void*>(static_cast<byte*>(currentAddress) + mbi.RegionSize);

                if (static_cast<byte*>(currentAddress) > static_cast<byte*>(hModule) + MAX_MODULE_SIZE) {
                    std::cerr << "Error: Exceeded maximum module size!" << std::endl;
                    return 0;
                }
            }
            return moduleSize;
        }

        void* scanPattern(HMODULE hModule, size_t moduleSize,
            const std::vector<byte>& pattern,
            const std::vector<byte>& mask) const {
            byte* address = reinterpret_cast<byte*>(hModule);
            const size_t patternSize = pattern.size();

            for (size_t i = 0; i < moduleSize - patternSize; ++i) {
                bool match = true;
                for (size_t j = 0; j < patternSize; ++j) {
                    if (mask[j] == 0xFF && address[i + j] != pattern[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return address + i;
                }
            }
            return nullptr;
        }

        std::string_view m_moduleName;
        std::string_view m_patternString;
    };
}