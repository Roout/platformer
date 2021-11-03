#include "Core.hpp"
#include <unordered_map>

namespace core {

CategoryName CategoryFromString(std::string_view str) {
    static const std::unordered_map<std::string_view, CategoryName> bindings {
        { "platform",    CategoryName::PLATFORM }, 
        { "border",      CategoryName::BORDER }, 
        { "spikes",      CategoryName::SPIKES }, 
        { "player",      CategoryName::PLAYER }, 
        { "props",       CategoryName::PROPS }, 
        { "enemy",       CategoryName::ENEMY }, 
        { "path",        CategoryName::PATH }, 
        { "influence",   CategoryName::INFLUENCE }, 
    };

    CategoryName category { core::CategoryName::UNDEFINED };
    if (auto it = bindings.find(str); it != bindings.cend()) {
        category = it->second;
    }

    return category;
}

} // namespace core 