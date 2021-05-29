#include "Arsenic/Arsenicpch.hpp"

#include "Arsenic/Renderer/VulkanContext.hpp"
#include "Arsenic/Renderer/Structure.hpp"
#include "Arsenic/Renderer/Shader.hpp"

#include "spirv_reflect.h"
#include "nlohmann/json.hpp"

namespace arsenic
{
    std::vector<char> loadSpvShaderFromFile(const char *spvFilePath)
    {
        std::ifstream file(spvFilePath, std::ios::ate | std::ios::binary);
        assert(file.is_open());

        const std::size_t size = static_cast<std::size_t>(file.tellg());
        file.seekg(0);

        std::vector<char> spvCode(size);
        file.read(spvCode.data(), size);
        
        return std::move(spvCode);
    }

    static VkShaderModule createShaderModule(const VulkanContext &renderContext, const std::vector<char> &spvCode)
    {
        VkShaderModuleCreateInfo shaderModuleCI = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCI.codeSize = static_cast<uint32_t>(spvCode.size());
        shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(spvCode.data());
        
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        checkVkResult(vkCreateShaderModule(renderContext.device, &shaderModuleCI, nullptr, &shaderModule));

        return shaderModule;
    }
    
    static ShaderEffect createShaderEffect(const VulkanContext renderContext, const std::initializer_list<SpvReflectShaderModule> &shaderReflectModules)
    {
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setLayoutBindings;

        for (const SpvReflectShaderModule &shaderReflectModule : shaderReflectModules)
        {
            std::vector<SpvReflectDescriptorSet*> reflectDescriptorSet;

            {
                uint32_t count = 0;
                SpvReflectResult result = spvReflectEnumerateDescriptorSets(&shaderReflectModule, &count, nullptr);
                assert(result == SPV_REFLECT_RESULT_SUCCESS);
                reflectDescriptorSet.resize(count);
                result = spvReflectEnumerateDescriptorSets(&shaderReflectModule, &count, reflectDescriptorSet.data());
                assert(result == SPV_REFLECT_RESULT_SUCCESS);

            }
            
            for (uint32_t i = 0; i != reflectDescriptorSet.size(); ++i) {
                const SpvReflectDescriptorSet &spvReflectSet = *(reflectDescriptorSet[i]);

                std::vector<VkDescriptorSetLayoutBinding> &layoutBindings = setLayoutBindings[spvReflectSet.set];
                layoutBindings.reserve(spvReflectSet.binding_count);

                for (uint32_t k = 0; k != spvReflectSet.binding_count; ++k) {
                    const SpvReflectDescriptorBinding &spvReflectDescriptorBinding = *(spvReflectSet.bindings[k]);

                    bool bindingExist = false;
                    
                    for (const VkDescriptorSetLayoutBinding &binding : layoutBindings) {
                        if (binding.binding == spvReflectDescriptorBinding.binding) {
                            bindingExist = true;
                        }

                        if (bindingExist) {
                            break;
                        }
                    }

                    if (!bindingExist) {
                        VkDescriptorSetLayoutBinding layoutBinding = {};
                        layoutBinding.binding = spvReflectDescriptorBinding.binding;
                        layoutBinding.descriptorCount = spvReflectDescriptorBinding.count;
                        layoutBinding.descriptorType = static_cast<VkDescriptorType>(spvReflectDescriptorBinding.descriptor_type);
                        layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
                        layoutBindings.emplace_back(layoutBinding);
                    }
                }
            }

        }
  

        ShaderEffect shaderEffect = {};

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(4);

        for (uint32_t i = 0; i != setLayoutBindings.size() && i != shaderEffect.descriptorSetLayouts.size(); ++i) {
            if (setLayoutBindings.find(i) == setLayoutBindings.end()) {
                continue;
            }

            const std::vector<VkDescriptorSetLayoutBinding> &layoutBindings = setLayoutBindings[i];

            VkDescriptorSetLayoutCreateInfo layoutCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            layoutCI.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutCI.pBindings = layoutBindings.data();

            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            checkVkResult(vkCreateDescriptorSetLayout(renderContext.device, &layoutCI, nullptr, &descriptorSetLayout));

            shaderEffect.descriptorSetLayouts[i] = descriptorSetLayout;
            descriptorSetLayouts.emplace_back(descriptorSetLayout);
        }

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
        pushConstantRange.size = 128;

        VkPipelineLayoutCreateInfo pipelineLayoutCI = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutCI.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        checkVkResult(vkCreatePipelineLayout(renderContext.device, &pipelineLayoutCI, nullptr, &pipelineLayout));
        shaderEffect.pipelineLayout = pipelineLayout;

        return std::move(shaderEffect);
    }

    ShaderEffect buildGraphicsShaderEffect(const VulkanContext &renderContext, const char *psoJsonFilePath)
    {
        std::ifstream file(psoJsonFilePath);
        assert(file.is_open());

        nlohmann::json psoJson;
        file >> psoJson;

        const std::string vertSpvFilePath = psoJson["vertSpvFilePath"].get<std::string>();
        const std::string fragSpvFilePath = psoJson["fragSpvFilePath"].get<std::string>();

        std::vector<char> vertSpvCode = loadSpvShaderFromFile(vertSpvFilePath.c_str());
        std::vector<char> fragSpvCode = loadSpvShaderFromFile(fragSpvFilePath.c_str());

        SpvReflectShaderModule vertSpvReflectModule = {};
        SpvReflectShaderModule fragSpvReflectModule = {};

        {
            SpvReflectResult result = spvReflectCreateShaderModule(static_cast<std::size_t>(vertSpvCode.size()), vertSpvCode.data(), &vertSpvReflectModule);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
        }

        {
            SpvReflectResult result = spvReflectCreateShaderModule(static_cast<std::size_t>(fragSpvCode.size()), fragSpvCode.data(), &fragSpvReflectModule);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
        }

        ShaderEffect shaderEffect = createShaderEffect(renderContext, {vertSpvReflectModule, fragSpvReflectModule});   
        shaderEffect.shaderStages.reserve(2);

        ShaderStage vertShaderStage = {};
        vertShaderStage.shaderModule = createShaderModule(renderContext, vertSpvCode);
        vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

        ShaderStage fragShaderStage = {};
        fragShaderStage.shaderModule = createShaderModule(renderContext, fragSpvCode);
        fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        shaderEffect.shaderStages.reserve(2);
        shaderEffect.shaderStages.emplace_back(vertShaderStage);
        shaderEffect.shaderStages.emplace_back(fragShaderStage);

        shaderEffect.psoJson = std::move(psoJson);  

        return std::move(shaderEffect);
    }

    ShaderEffect buildComputeShaderEffect(const VulkanContext &renderContext, const char *compSpvFilePath)
    {
        std::vector<char> computeSpvCode = loadSpvShaderFromFile(compSpvFilePath);

        SpvReflectShaderModule computeSpvReflectModule = {};

        {
            SpvReflectResult result = spvReflectCreateShaderModule(static_cast<std::size_t>(computeSpvCode.size()), computeSpvCode.data(), &computeSpvReflectModule);
            assert(result == SPV_REFLECT_RESULT_SUCCESS);
        }

        ShaderEffect shaderEffect = createShaderEffect(renderContext, {computeSpvReflectModule});     

        ShaderStage computeShaderStage = {};
        computeShaderStage.shaderModule = createShaderModule(renderContext, computeSpvCode);
        computeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        
        shaderEffect.shaderStages.emplace_back(computeShaderStage);

        return std::move(shaderEffect);
    }

    ShaderPass buildGraphicsShaderPass(const VulkanContext &renderContext, const VkRenderPass renderpass, const uint32_t subpassIndex, ShaderEffect *pShaderEffect)
    {
        assert(pShaderEffect);

        VkVertexInputBindingDescription vertexInputBindingDesc = {};
        vertexInputBindingDesc.binding = 0;
        vertexInputBindingDesc.inputRate = (pShaderEffect->psoJson["inputRate"].get<std::string>() == "vertex") ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
        vertexInputBindingDesc.stride = sizeof(Vertex);

        // TODO: use spv_reflect for this
        std::array<VkVertexInputAttributeDescription, 3> vertexAttribDescs = {};
        vertexAttribDescs[0].binding = 0;
        vertexAttribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexAttribDescs[0].location = 0;
        vertexAttribDescs[0].offset = offsetof(Vertex, Vertex::position);

        vertexAttribDescs[1].binding = 0;
        vertexAttribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        vertexAttribDescs[1].location = 1;
        vertexAttribDescs[1].offset = offsetof(Vertex, Vertex::normal);
        
        vertexAttribDescs[2].binding = 0;
        vertexAttribDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
        vertexAttribDescs[2].location = 2;
        vertexAttribDescs[2].offset = offsetof(Vertex, Vertex::uv);
        
        VkPipelineVertexInputStateCreateInfo vertexInputCI = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

        if (pShaderEffect->psoJson["enableVertexAttribs"].get<bool>()) {
            vertexInputCI.vertexBindingDescriptionCount = 1;
            vertexInputCI.pVertexBindingDescriptions = &vertexInputBindingDesc;
            vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribDescs.size());
            vertexInputCI.pVertexAttributeDescriptions = vertexAttribDescs.data();
        }

        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblerCI = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssemblerCI.primitiveRestartEnable = VK_FALSE;
        
        if (const std::string topology = pShaderEffect->psoJson["topology"].get<std::string>(); topology == "triangle") {
            inputAssemblerCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        VkPipelineRasterizationStateCreateInfo rasterStateCI = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterStateCI.rasterizerDiscardEnable = VK_FALSE;
        rasterStateCI.lineWidth = 1.0f;
        rasterStateCI.cullMode = VK_CULL_MODE_NONE;
        rasterStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterStateCI.polygonMode = VK_POLYGON_MODE_FILL;

        if (const std::string cullMode = pShaderEffect->psoJson["cullMode"].get<std::string>(); cullMode == "back") {
            rasterStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
        }
        else if (cullMode == "front") {
            rasterStateCI.cullMode = VK_CULL_MODE_FRONT_BIT;
        }

        if (const std::string frontFace = pShaderEffect->psoJson["frontFace"].get<std::string>(); frontFace == "clockWise") {
            rasterStateCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
        }

        if (const std::string polygonMode = pShaderEffect->psoJson["polygonMode"].get<std::string>(); polygonMode == "wireframe") {
            rasterStateCI.polygonMode = VK_POLYGON_MODE_LINE;
        }

        VkPipelineMultisampleStateCreateInfo multiSampleCI = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multiSampleCI.sampleShadingEnable = VK_FALSE;
        multiSampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachStates;
        const std::size_t numColorAttachment = pShaderEffect->psoJson["colorAttachmentDescs"].size();
        colorBlendAttachStates.reserve(numColorAttachment);

        for (auto colorAttachmentJson : pShaderEffect->psoJson["colorAttachmentDescs"]) {
            VkPipelineColorBlendAttachmentState colorBlendState = {};
            colorBlendState.blendEnable = VK_FALSE;
            colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

            if (int numColorComponent = colorAttachmentJson["numColorComponent"].get<int>(); numColorComponent == 1) {
                colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
            }
            else if (numColorComponent == 2) {
                colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
            }
            else if (numColorComponent == 3) {
                colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
            }

            colorBlendAttachStates.emplace_back(colorBlendState);
        }

        VkPipelineColorBlendStateCreateInfo colorBlendCI = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        colorBlendCI.attachmentCount = static_cast<uint32_t>(colorBlendAttachStates.size());
        colorBlendCI.pAttachments = colorBlendAttachStates.data();
        colorBlendCI.logicOpEnable = VK_FALSE;
        
        VkPipelineDepthStencilStateCreateInfo depthStencilCI = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        depthStencilCI.depthTestEnable = pShaderEffect->psoJson["depthTest"].get<bool>() ? VK_TRUE : VK_FALSE;
        depthStencilCI.depthWriteEnable = VK_TRUE;
        depthStencilCI.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;  

        VkViewport viewport = {};
        VkRect2D scissor = {};
        
        VkPipelineViewportStateCreateInfo viewportStateCI = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportStateCI.viewportCount = 1;
        viewportStateCI.pViewports = &viewport;
        viewportStateCI.scissorCount = 1;
        viewportStateCI.pScissors = &scissor;

        std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicStateCI = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
        dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCI.pDynamicStates = dynamicStates.data();
        
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCIS;
        shaderStageCIS.reserve(pShaderEffect->shaderStages.size());

        for (const ShaderStage &shaderStage : pShaderEffect->shaderStages) {
            assert(shaderStage.shaderModule);

            VkPipelineShaderStageCreateInfo shaderStageCI = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            shaderStageCI.module = shaderStage.shaderModule;
            shaderStageCI.stage = shaderStage.stage;
            shaderStageCI.pName = "main";
            shaderStageCI.pSpecializationInfo = nullptr;

            shaderStageCIS.emplace_back(shaderStageCI);
        }
        
        VkGraphicsPipelineCreateInfo pipelineCI = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        pipelineCI.stageCount = static_cast<uint32_t>(shaderStageCIS.size());
        pipelineCI.pStages = shaderStageCIS.data();
        pipelineCI.pVertexInputState = &vertexInputCI;
        pipelineCI.pInputAssemblyState = &inputAssemblerCI;
        pipelineCI.pRasterizationState = &rasterStateCI;
        pipelineCI.pMultisampleState = &multiSampleCI;
        pipelineCI.pColorBlendState = &colorBlendCI;
        pipelineCI.pDepthStencilState = &depthStencilCI;
        pipelineCI.pTessellationState = nullptr;
        pipelineCI.pViewportState = &viewportStateCI;
        pipelineCI.pDynamicState = &dynamicStateCI;
        pipelineCI.layout = pShaderEffect->pipelineLayout;
        pipelineCI.renderPass = renderpass;
        pipelineCI.subpass = subpassIndex;
        pipelineCI.basePipelineHandle = VK_NULL_HANDLE;

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkVkResult(vkCreateGraphicsPipelines(renderContext.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));

        ShaderPass shaderPass = {};
        shaderPass.pipeline = pipeline;
        shaderPass.pShaderEffect = pShaderEffect;

        return shaderPass;
    }

    ShaderPass buildComputeShaderPass(const VulkanContext &renderContext, ShaderEffect *pShaderEffect)
    {
        assert(pShaderEffect);

        VkPipelineShaderStageCreateInfo shaderStageCI = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        shaderStageCI.module = pShaderEffect->shaderStages[0].shaderModule;
        shaderStageCI.stage = pShaderEffect->shaderStages[0].stage;
        shaderStageCI.pName = "main";
        
        VkComputePipelineCreateInfo pipelineCI = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineCI.layout = pShaderEffect->pipelineLayout;
        pipelineCI.stage = shaderStageCI;
        pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCI.basePipelineIndex = -1;

        VkPipeline pipeline = VK_NULL_HANDLE;
        checkVkResult(vkCreateComputePipelines(renderContext.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));

        ShaderPass shaderPass = {};
        shaderPass.pipeline = pipeline;
        shaderPass.pShaderEffect = pShaderEffect;

        return shaderPass;
    }
}