#pragma once

#include "Arsenic/Renderer/VulkanHeader.hpp"

#include "nlohmann/json.hpp"

namespace arsenic
{
    struct VulkanContext;

    struct ShaderStage 
    {
        VkShaderModule shaderModule;
        VkShaderStageFlagBits stage;
    };

    struct ShaderEffect
    {
        VkPipelineLayout pipelineLayout;
        std::array<VkDescriptorSetLayout, 4> descriptorSetLayouts;
        std::vector<ShaderStage> shaderStages;
        nlohmann::json psoJson;
    };

    struct ShaderPass
    {
        ShaderEffect *pShaderEffect;
        VkPipeline pipeline;
    };

    std::vector<char> loadSpvShaderFromFile(const char *spvFilePath);

    ShaderEffect buildGraphicsShaderEffect(const VulkanContext &renderContext, const char *psoJsonFilePath);
    ShaderEffect buildComputeShaderEffect(const VulkanContext &renderContext, const char *compSpvFilePath);

    ShaderPass buildGraphicsShaderPass(const VulkanContext &renderContext, const VkRenderPass renderpass, const uint32_t subpassIndex, ShaderEffect *pShaderEffect);
    ShaderPass buildComputeShaderPass(const VulkanContext &renderContext, ShaderEffect *pShaderEffect);
}