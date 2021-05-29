#pragma once

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>

namespace arsenic
{
    enum class Handle : uint32_t {};
    
    constexpr Handle invalidHandle = static_cast<Handle>(0);

    using BufferHandle = Handle;
    using TextureHandle = Handle;
    using MeshHandle = Handle;
    using MaterialHandle = Handle;
}