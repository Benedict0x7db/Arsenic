#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace arsenic
{
    struct Version
    {
        uint32_t major = 0;
        uint32_t minor = 0;
        uint32_t patch = 0;
    };

    struct Extent2D
    {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct ApplicationDesc
    {
        std::string appName = "silicon";
        Version version;
        Extent2D windowExtent;
    };
    
    template<typename T>
    using RefCountPtr = std::shared_ptr<T>;

    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T, typename ...Args>
    inline RefCountPtr<T> createRefCountPtr(Args &&...args)    
    { 
        return std::make_shared<T>(std::forward<Args>(args)...); 
    }

    template<typename T, typename ...Args>
    inline UniquePtr<T> createUniquePtr(Args &&...args) 
    { 
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}