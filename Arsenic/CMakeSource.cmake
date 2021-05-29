set(EXTERNAL_SOURCE
${CMAKE_SOURCE_DIR}/External/imgui/imgui.cpp
${CMAKE_SOURCE_DIR}/External/imgui/imgui.h
${CMAKE_SOURCE_DIR}/External/imgui/imgui_demo.cpp
${CMAKE_SOURCE_DIR}/External/imgui/imgui_draw.cpp
${CMAKE_SOURCE_DIR}/External/imgui/imgui_internal.h
${CMAKE_SOURCE_DIR}/External/imgui/imgui_widgets.cpp
${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_glfw.h
${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_glfw.cpp
${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.h
${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.cpp

${CMAKE_SOURCE_DIR}/External/vma/vk_mem_alloc.cpp
${CMAKE_SOURCE_DIR}/External/vma/vk_mem_alloc.hpp

${CMAKE_SOURCE_DIR}/External/spv_reflect/spirv_reflect.h
${CMAKE_SOURCE_DIR}/External/spv_reflect/spirv_reflect.cc

${CMAKE_SOURCE_DIR}/External/stb/stb_image.cpp
${CMAKE_SOURCE_DIR}/External/stb/stb_image.hpp
${CMAKE_SOURCE_DIR}/External/stb/stb_image_write.hpp)

set(ARSENIC_SOURCE
${EXTERNAL_SOURCE}

"Source/Arsenic/Event/ApplicationEvents.hpp"
"Source/Arsenic/Event/Event.hpp"
"Source/Arsenic/Event/KeyEvents.hpp"
"Source/Arsenic/Event/MouseEvents.hpp"

"Source/Arsenic/Math/ClipSpace.hpp"
"Source/Arsenic/Math/MatrixConvert.hpp"
"Source/Arsenic/Math/Mat4.hpp"
"Source/Arsenic/Math/Mat3.hpp"
"Source/Arsenic/Math/Math.hpp"
"Source/Arsenic/Math/MathFunctions.hpp"
"Source/Arsenic/Math/MatrixTransforms.hpp"
"Source/Arsenic/Math/Vec2.hpp"
"Source/Arsenic/Math/Vec3.hpp"
"Source/Arsenic/Math/Vec4.hpp"

"Source/Arsenic/Renderer/Camera.cpp"
"Source/Arsenic/Renderer/Camera.hpp"
"Source/Arsenic/Renderer/MaterialManager.hpp"
"Source/Arsenic/Renderer/MaterialManager.cpp"
"Source/Arsenic/Renderer/Shader.hpp"
"Source/Arsenic/Renderer/Shader.cpp"
"Source/Arsenic/Renderer/Structure.hpp"
"Source/Arsenic/Renderer/Handle.hpp"
"Source/Arsenic/Renderer/Instance.cpp"
"Source/Arsenic/Renderer/Instance.hpp"
"Source/Arsenic/Renderer/PhysicalDevice.cpp"
"Source/Arsenic/Renderer/PhysicalDevice.hpp"
"Source/Arsenic/Renderer/VulkanContext.cpp"
"Source/Arsenic/Renderer/VulkanContext.hpp"
"Source/Arsenic/Renderer/Swapchain.cpp"
"Source/Arsenic/Renderer/Swapchain.hpp"
"Source/Arsenic/Renderer/VulkanHeader.hpp"

"Source/Arsenic/Scene/Component.hpp"
"Source/Arsenic/Scene/Component.cpp"

"Source/Arsenic/Scene/Entity.hpp"
"Source/Arsenic/Scene/Scene.cpp"
"Source/Arsenic/Scene/Scene.hpp"

"Source/Arsenic/Arsenicpch.hpp"
"Source/Arsenic/Core/EntryPoint.cpp"
"Source/Arsenic/Core/Application.cpp"
"Source/Arsenic/Core/Application.hpp"
"Source/Arsenic/Core/Application.inl"
"Source/Arsenic/Core/Input.cpp"
"Source/Arsenic/Core/Input.hpp"
"Source/Arsenic/Core/Keycode.hpp"
"Source/Arsenic/Core/Layer.hpp"
"Source/Arsenic/Core/Logger.cpp"
"Source/Arsenic/Core/Logger.hpp"
"Source/Arsenic/Core/Mousecode.hpp"
"Source/Arsenic/Core/Utils.hpp"
"Source/Arsenic/Core/Window.cpp"
"Source/Arsenic/Core/Window.hpp")