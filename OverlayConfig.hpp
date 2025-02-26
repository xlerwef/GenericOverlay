// OverlayConfig.hpp
#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include "include/json.hpp"

namespace Overlay {
    struct SignaturePattern {
        std::string name;
        std::string pattern;
        std::string module;

        SignaturePattern(const std::string& n, const std::string& p, const std::string& m)
            : name(n), pattern(p), module(m) {}
    };

    class OverlayConfig {
    public:
        static OverlayConfig& getInstance() {
            static OverlayConfig instance;
            return instance;
        }

        bool loadFromFile(const std::string& filename);
        bool loadFromString(const std::string& jsonStr);

        std::optional<SignaturePattern> getPattern(const std::string& name) const;
        bool addPattern(const SignaturePattern& pattern);

        // Default patterns that work with most games
        static const inline std::string DEFAULT_PRESENT_PATTERN = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 20 41";
        static const inline std::string DEFAULT_CREATE_PATTERN = "48 89 5C 24 ? 57 48 83 EC ? 33 C0";

    private:
        OverlayConfig() = default;
        std::unordered_map<std::string, SignaturePattern> patterns;
    };
}
