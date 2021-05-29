#include "SandboxLayer.hpp"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace arsenic
{
    SandboxLayer::SandboxLayer()
    {      
        _vulkanContext = createVulkanContext(Application::getWindow().getNativeWindowHandle(), "Sandbox");
        _materialManager.initialize(_vulkanContext);
        initializeFrame();

        _renderTargetExtent.width = 1280;
        _renderTargetExtent.height = 720;
        createRenderTarget();

        _swapchain = createSwapchain(_vulkanContext.device, _vulkanContext.surface, _vulkanContext.physicalDevice.vkPhysicalDevice, _renderTargetExtent);
        createDepthTexture();
        createRenderPass();
        createFramebuffers();
        setupImGui();
        setupShaderResource();

        _sceneEnviromentMap = _vulkanContext.createImageCube(TextureFormat::RGBAU8, "Assets/Scene/enviromentMap.json");
        _generalSampler = _materialManager.createSampler(_vulkanContext);

        _rtShaderEffect = buildComputeShaderEffect(_vulkanContext, "Assets/Shaders/Spv/rtCompute.comp.spv");
        _fullScreenShaderEffect = buildGraphicsShaderEffect(_vulkanContext, "Assets/Pso/fullScreenPSO.json");

        _rtShaderPass = buildComputeShaderPass(_vulkanContext, &_rtShaderEffect);
        _fullScreenPass = buildGraphicsShaderPass(_vulkanContext, _renderpass, 0, &_fullScreenShaderEffect);

        createEngineDescriptorPool();
        setupGlobalDescriptorSet();

        _camera.initialize(_swapchain.imageExtent.width, _swapchain.imageExtent.height);
        _cameraBuffer.proj = _camera.projMatrix;
        _cameraBuffer.invProj = math::inverse(_camera.projMatrix);

        _dirLightEntity = _scene.createEntity();
        _dirLightEntity.addComponent<DirectionLight>();
    }       
    
    SandboxLayer::~SandboxLayer()
    {      
        checkVkResult(vkDeviceWaitIdle(_vulkanContext.device));

        deInitializeFrame();
        destroyFramebuffers();
        destroyDepthTexture();
        destroyRenderTarget();
        vkDestroyRenderPass(_vulkanContext.device, _renderpass, nullptr);
        destroySwapchain(_vulkanContext.device, _swapchain);
        vkDestroyDescriptorPool(_vulkanContext.device, _imguiDescriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void SandboxLayer::onUpdate(const float dt) 
    {
        _camera.update(dt);

        _cameraBuffer.cameraPos = _camera.position;
        _cameraBuffer.fov = _camera.fov;
        _cameraBuffer.aspect = _camera.aspect;
        _cameraBuffer.znear = _camera.znear;
        _cameraBuffer.view = _camera.viewMatrix;
        _cameraBuffer.invView = math::inverse(_camera.viewMatrix);
    }
    
    void SandboxLayer::onRender()
    {
        _renderObjects.clear();
        _meshObjects.clear();
        _lights.clear();
        _materials.clear();
        
        auto &registry = _scene.getRegistry();

        {
            auto view = registry.view<Mesh, Material, Transform>();

            view.each([this](const auto entity, const Mesh &mesh, const Material &material, const Transform &transform) {
                _materials.emplace_back(material);

                RenderObject renderObject = {};
                renderObject.transformMatrix = transform.getModelMatrix();
                renderObject.materialIndex = _materials.size() - 1;
                _renderObjects.emplace_back(renderObject);

                MeshObject meshObject = {};
                meshObject.pMesh = &mesh;
                meshObject.renderObjectIndex = _renderObjects.size() - 1;
                _meshObjects.emplace_back(meshObject);
            });
        }

        {
            auto view = registry.view<DirectionLight, Transform>();
            
            view.each([this](const auto entity,  const DirectionLight &dirLight, const Transform &transform) {
                math::mat4f transformMatrix = transform.getModelMatrix();

                Light light = {};
                light.type = 0;
                light.color = dirLight.color * dirLight.intensity;
                light.position = transformMatrix * math::vec4f(transform.position, 0.0f);
                _lights.emplace_back(light);
            });   
        } 
    }
    
    void SandboxLayer::onImGuiRender()
    {
        static constexpr float dragSpeed = 0.1f;
        static constexpr float dragMin = -std::numeric_limits<float>::max();
        static constexpr float dragMax = std::numeric_limits<float>::max();

        ImGui::Begin("Scene");

        {
            ImGui::Text("Camera");
            
            ImGui::DragFloat3("Position ##camera", &_camera.position.x, dragSpeed, dragMin, dragMax);
            ImGui::DragFloat("Fov ##camera", &_camera.fov, dragSpeed, 1.0f, 179.0f);

            _camera.updateProjMatrix();
            _cameraBuffer.proj = _cameraBuffer.proj;
            _cameraBuffer.invProj = math::inverse(_cameraBuffer.proj);
        }
        ImGui::Separator();
        {
            Transform &transform = _dirLightEntity.getComponent<Transform>();
            DirectionLight &dirLight = _dirLightEntity.getComponent<DirectionLight>();

            ImGui::Text("Direction Light");
            
            ImGui::DragFloat3("Direction ##Direction Light", &transform.position.x, dragSpeed, dragMin, dragMax);
            ImGui::DragFloat3("Rotation ##Direction Light", &transform.rotation.x, dragSpeed, dragMin, dragMax);
            ImGui::ColorEdit3("Color ##Direction Light", &dirLight.color.x);
            ImGui::DragFloat("Intensity ##Direction Light", &dirLight.intensity, dragSpeed, dragMin,dragMax);
        }

        ImGui::End();
    }
    
    void SandboxLayer::onEvent(Event &e)
    {
        EventDispatcher dispatcher(e);

        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &event) {
            int width = static_cast<int>(event.width);
            int height = static_cast<int>(event.height);

            while (width == 0 || height == 0) {
                glfwWaitEvents();
                glfwGetFramebufferSize(Application::getWindow().getNativeWindowHandle(), &width, &height);
            }

            recreateSwapchain({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
            return false;
        });

        _camera.onEvent(e);
        _cameraBuffer.proj = _camera.projMatrix;
        _cameraBuffer.invProj = math::inverse(_camera.projMatrix);
    }

    void SandboxLayer::onFrameBegin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void SandboxLayer::onFrameEnd() 
    {
        Frame &frame = getCurrentFrame();

        checkVkResult(vkWaitForFences(_vulkanContext.device, 1, &frame.frameInFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
        checkVkResult(vkResetFences(_vulkanContext.device, 1, &frame.frameInFlightFence));

        uint32_t imageIndex = acquireImageFromSwapchain(_vulkanContext.device, frame.imageReadySemaphore, _swapchain);
        
        checkVkResult(vkResetCommandPool(_vulkanContext.device, frame.commandPool, 0));
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        {
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            commandBufferAllocateInfo.commandBufferCount = 1;
            commandBufferAllocateInfo.commandPool = frame.commandPool;
            checkVkResult(vkAllocateCommandBuffers(_vulkanContext.device, &commandBufferAllocateInfo, &commandBuffer));
        }

        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        {
            
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = _renderTarget.vkImage;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = 0;
            imageMemoryBarrier.oldLayout = _renderTarget.imageLayout;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = 1;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        
            _renderTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        const VulkanBuffer &gpuRenderObjectBuffer = _gpuRenderObjectBuffers.value[_currentFrame];
        const VulkanBuffer &gpuLightBuffer = _gpuLightBuffers.value[_currentFrame];
        const VulkanBuffer &gpuMaterialBuffer = _gpuMaterialBuffers.value[_currentFrame];
    
        for (std::size_t i = 0; i != _renderObjects.size() && i != maxRenderObjects; ++i) {
            std::memcpy(gpuRenderObjectBuffer.pMappedPointer + i * sizeof(RenderObject), &_renderObjects[i], sizeof(RenderObject));
            ++_sceneBuffer.numLights;
        }

                
        for (std::size_t i = 0; i != _lights.size() && i != maxLights; ++i) {
            std::memcpy(gpuLightBuffer.pMappedPointer + i * sizeof(Light), &_lights[i], sizeof(Light));
        }

        for (std::size_t i = 0; i != _materials.size() && i != maxMaterials; ++i) {
            std::memcpy(gpuMaterialBuffer.pMappedPointer + i * sizeof(Material), &_materials[i], sizeof(Material));
        }
        
        const VulkanBuffer &gpuSceneBuffer = _gpuSceneBuffers.value[_currentFrame];
        const VulkanBuffer &gpuCameraBuffer =_gpuCameraBuffers.value[_currentFrame];

        std::memcpy(gpuSceneBuffer.pMappedPointer, &_sceneBuffer, sizeof(SceneBuffer));
        std::memcpy(gpuCameraBuffer.pMappedPointer, &_cameraBuffer, sizeof(CameraBuffer));

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _rtShaderPass.pShaderEffect->pipelineLayout,
                        0, 1, &_globalDescriptorSet.value[_currentFrame], 0, nullptr);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _rtShaderPass.pipeline);

        int groupCountX = _renderTargetExtent.width / 32 + 1;
        int groupCountY = _renderTargetExtent.height / 32 + 1; 
        vkCmdDispatch(commandBuffer, groupCountX, groupCountY, 1);

        {
            VkImageMemoryBarrier imageMemoryBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = _renderTarget.vkImage;
            imageMemoryBarrier.oldLayout = _renderTarget.imageLayout;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;
            imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            
            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
                        0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            _renderTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[0].depthStencil = {0.0f, 1};
        
        VkRenderPassBeginInfo renderPassBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        renderPassBeginInfo.renderPass = _renderpass;
        renderPassBeginInfo.renderArea.extent = _swapchain.imageExtent;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.framebuffer = _frameBuffers[imageIndex];

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.y = static_cast<float>(_swapchain.imageExtent.height);
        viewport.width = static_cast<float>(_swapchain.imageExtent.width);
        viewport.height = -static_cast<float>(_swapchain.imageExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.extent = _swapchain.imageExtent;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _fullScreenPass.pShaderEffect->pipelineLayout,
                        0, 1, &_globalDescriptorSet.value[_currentFrame], 0, nullptr);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _fullScreenPass.pipeline);

        vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        ImGui::Render();
        ImDrawData* pDrawData = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(pDrawData, commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        checkVkResult(vkEndCommandBuffer(commandBuffer));
    
        {
            static constexpr VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
            VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &frame.imageReadySemaphore;
            submitInfo.pWaitDstStageMask = &waitStage;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &frame.renderFinishSemaphore;

            checkVkResult(vkQueueSubmit(_vulkanContext.graphicsQueue, 1, &submitInfo, frame.frameInFlightFence));
        }

        presentSwapchainImage(_vulkanContext.device, _vulkanContext.presentQueue, frame.renderFinishSemaphore, imageIndex, _swapchain);
        _currentFrame = (_currentFrame + 1) % maxFrameInFlight;
    }

    void SandboxLayer::setupShaderResource()
    {
        for (std::size_t i = 0; i != maxFrameInFlight; ++i) {
            BufferDesc sceneBufferDesc = {};
            sceneBufferDesc.size = sizeof(SceneBuffer);
            sceneBufferDesc.type = BufferType::Uniform;
            sceneBufferDesc.usage = BufferUsage::Dynamic;

            _gpuSceneBuffers.value[i] = _vulkanContext.createBuffer(sceneBufferDesc, nullptr);

            BufferDesc cameraBufferDesc = {};
            cameraBufferDesc.size = sizeof(CameraBuffer);
            cameraBufferDesc.type = BufferType::Uniform;
            cameraBufferDesc.usage = BufferUsage::Dynamic;  

            _gpuCameraBuffers.value[i] = _vulkanContext.createBuffer(cameraBufferDesc, nullptr);

            BufferDesc renderObjectBufferDesc = {};
            renderObjectBufferDesc.size = sizeof(RenderObject) * maxRenderObjects;
            renderObjectBufferDesc.type = BufferType::Storage;
            renderObjectBufferDesc.usage = BufferUsage::Dynamic;

            _gpuRenderObjectBuffers.value[i] = _vulkanContext.createBuffer(renderObjectBufferDesc, nullptr);
        
            BufferDesc lightBufferDesc = {};
            lightBufferDesc.size = sizeof(Light) * maxLights;
            lightBufferDesc.type = BufferType::Storage;
            lightBufferDesc.usage = BufferUsage::Dynamic;

            _gpuLightBuffers.value[i] = _vulkanContext.createBuffer(lightBufferDesc, nullptr);


            BufferDesc materialBufferDesc = {};
            materialBufferDesc.size = sizeof(Material) * maxMaterials;
            materialBufferDesc.type = BufferType::Storage;
            materialBufferDesc.usage = BufferUsage::Dynamic;

            _gpuMaterialBuffers.value[i] = _vulkanContext.createBuffer(materialBufferDesc, nullptr);
        }        
    }

    void SandboxLayer::createEngineDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 6> poolSizes = {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 100},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100}
        };

        VkDescriptorPoolCreateInfo descriptorPoolCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolCI.pPoolSizes = poolSizes.data();
        descriptorPoolCI.maxSets = maxFrameInFlight;

        checkVkResult(vkCreateDescriptorPool(_vulkanContext.device, &descriptorPoolCI, nullptr, &_descriptorPool));
    }

    void SandboxLayer::setupGlobalDescriptorSet()
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayout(maxFrameInFlight, _rtShaderPass.pShaderEffect->descriptorSetLayouts[0]);

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = maxFrameInFlight;
        descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayout.data();
        
        checkVkResult(vkAllocateDescriptorSets(_vulkanContext.device, &descriptorSetAllocateInfo, _globalDescriptorSet.value.data()));

        for (std::size_t i = 0; i != maxFrameInFlight; ++i) {
            const VulkanBuffer &gpuSceneBuffer = _gpuSceneBuffers.value[i];
            const VulkanBuffer &gpuCameraBuffer = _gpuCameraBuffers.value[i];
            const VulkanBuffer &gpuRenderObjectBuffer = _gpuRenderObjectBuffers.value[i];
            const VulkanBuffer &gpuLightBuffer = _gpuLightBuffers.value[i];
            const VulkanBuffer &gpuMaterialBuffer = _gpuMaterialBuffers.value[i];

            std::array<VkWriteDescriptorSet, 8> writeDescriptors = {};

            VkDescriptorBufferInfo gpuSceneDescriptorBufferInfo = {};
            gpuSceneDescriptorBufferInfo.buffer = gpuSceneBuffer.vkBuffer;
            gpuSceneDescriptorBufferInfo.range = gpuSceneBuffer.size;

            writeDescriptors[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors[0].dstSet = _globalDescriptorSet.value[i];
            writeDescriptors[0].dstBinding = 0;
            writeDescriptors[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptors[0].descriptorCount = 1;
            writeDescriptors[0].dstArrayElement = 0;
            writeDescriptors[0].pBufferInfo = &gpuSceneDescriptorBufferInfo;

            VkDescriptorBufferInfo gpuCameraDescriptorBufferInfo = {};
            gpuCameraDescriptorBufferInfo.buffer = gpuCameraBuffer.vkBuffer;
            gpuCameraDescriptorBufferInfo.range = gpuCameraBuffer.size;

            writeDescriptors[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors[1].dstSet = _globalDescriptorSet.value[i];
            writeDescriptors[1].dstBinding = 1;
            writeDescriptors[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptors[1].descriptorCount = 1;
            writeDescriptors[1].dstArrayElement = 0;
            writeDescriptors[1].pBufferInfo = &gpuCameraDescriptorBufferInfo;

            VkDescriptorBufferInfo gpuRenderObjectDescriptorBufferInfo = {};
            gpuRenderObjectDescriptorBufferInfo.buffer = gpuRenderObjectBuffer.vkBuffer;
            gpuRenderObjectDescriptorBufferInfo.range = gpuRenderObjectBuffer.size;

            writeDescriptors[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors[3].dstSet = _globalDescriptorSet.value[i];
            writeDescriptors[3].dstBinding = 2;
            writeDescriptors[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptors[3].descriptorCount = 1;
            writeDescriptors[3].dstArrayElement = 0;
            writeDescriptors[3].pBufferInfo = &gpuRenderObjectDescriptorBufferInfo;

            VkDescriptorBufferInfo gpuLightDescriptorBufferInfo = {};
            gpuLightDescriptorBufferInfo.buffer = gpuLightBuffer.vkBuffer;
            gpuLightDescriptorBufferInfo.range = gpuLightBuffer.size;

            writeDescriptors[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors[2].dstSet = _globalDescriptorSet.value[i];
            writeDescriptors[2].dstBinding = 3;
            writeDescriptors[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptors[2].descriptorCount = 1;
            writeDescriptors[2].dstArrayElement = 0;
            writeDescriptors[2].pBufferInfo = &gpuLightDescriptorBufferInfo;

            VkDescriptorBufferInfo gpuMaterialDescriptorBufferInfo = {};
            gpuMaterialDescriptorBufferInfo.buffer = gpuMaterialBuffer.vkBuffer;
            gpuMaterialDescriptorBufferInfo.range = gpuMaterialBuffer.size;

            writeDescriptors[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptors[4].dstSet = _globalDescriptorSet.value[i];
            writeDescriptors[4].dstBinding = 4;
            writeDescriptors[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptors[4].descriptorCount = 1;
            writeDescriptors[4].dstArrayElement = 0;
            writeDescriptors[4].pBufferInfo = &gpuMaterialDescriptorBufferInfo;
            
            {
                VkDescriptorImageInfo descriptorImageInfo = {};
                descriptorImageInfo.imageView = _renderTarget.vkImageView;
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                
                writeDescriptors[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptors[5].dstSet = _globalDescriptorSet.value[i];
                writeDescriptors[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writeDescriptors[5].dstBinding = 5;
                writeDescriptors[5].descriptorCount = 1;
                writeDescriptors[5].dstArrayElement = 0;
                writeDescriptors[5].pImageInfo = &descriptorImageInfo;
            }
            
            {
                VkDescriptorImageInfo descriptorImageInfo = {};
                descriptorImageInfo.imageView = _renderTarget.vkImageView;
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptorImageInfo.sampler = _generalSampler;
                
                writeDescriptors[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptors[6].dstSet = _globalDescriptorSet.value[i];
                writeDescriptors[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptors[6].dstBinding = 6;
                writeDescriptors[6].descriptorCount = 1;
                writeDescriptors[6].dstArrayElement = 0;
                writeDescriptors[6].pImageInfo = &descriptorImageInfo;
            }

            {
                VkDescriptorImageInfo descriptorImageInfo = {};
                descriptorImageInfo.imageLayout = _sceneEnviromentMap.imageLayout;
                descriptorImageInfo.imageView = _sceneEnviromentMap.vkImageView;
                descriptorImageInfo.sampler = _generalSampler;

                writeDescriptors[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptors[7].dstSet = _globalDescriptorSet.value[i];
                writeDescriptors[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                writeDescriptors[7].dstBinding = 7;
                writeDescriptors[7].descriptorCount = 1;
                writeDescriptors[7].dstArrayElement = 0;
                writeDescriptors[7].pImageInfo = &descriptorImageInfo;
            }

            vkUpdateDescriptorSets(_vulkanContext.device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
        }
    }

    void SandboxLayer::setupPerPassDescriptorSet()
    {   
    }

    void SandboxLayer::initializeFrame()
    {
        for (Frame &frame : _frames.value) {
            VkFenceCreateInfo fenceCI = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            checkVkResult(vkCreateFence(_vulkanContext.device, &fenceCI, nullptr, &frame.frameInFlightFence));

            VkSemaphoreCreateInfo semaphoreCI = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            checkVkResult(vkCreateSemaphore(_vulkanContext.device, &semaphoreCI, nullptr, &frame.imageReadySemaphore));
            checkVkResult(vkCreateSemaphore(_vulkanContext.device, &semaphoreCI, nullptr, &frame.renderFinishSemaphore));

            VkCommandPoolCreateInfo commandPoolCI = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
            commandPoolCI.queueFamilyIndex = _vulkanContext.physicalDevice.queueFamilies.graphicsFamily.value();
            checkVkResult(vkCreateCommandPool(_vulkanContext.device, &commandPoolCI, nullptr, &frame.commandPool));
        }
    }

    void SandboxLayer::deInitializeFrame()
    {
        for (Frame &frame : _frames.value) {
            vkDestroyFence(_vulkanContext.device, frame.frameInFlightFence, nullptr);
            vkDestroySemaphore(_vulkanContext.device, frame.imageReadySemaphore, nullptr);
            vkDestroySemaphore(_vulkanContext.device, frame.renderFinishSemaphore, nullptr);
            vkDestroyCommandPool(_vulkanContext.device, frame.commandPool, nullptr);
        }
    }

    void SandboxLayer::createRenderTarget()
    {
        const VkFormat format = _vulkanContext.findSupportedFormat({VK_FORMAT_R8G8B8A8_SNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
        _renderTarget.vkFormat = format;
        _renderTarget.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.extent = { _renderTargetExtent.width, _renderTargetExtent.height, 1 };
        imageCI.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCI.format = format;
        imageCI.arrayLayers = 1;
        imageCI.mipLevels = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        checkVkResult(vmaCreateImage(_vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &_renderTarget.vkImage, &_renderTarget.vmaAllocation, nullptr));

        VkImageViewCreateInfo imageviewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageviewCI.image = _renderTarget.vkImage;
        imageviewCI.format = _renderTarget.vkFormat;
        imageviewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageviewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageviewCI.subresourceRange.baseArrayLayer = 0;
        imageviewCI.subresourceRange.layerCount = 1;
        imageviewCI.subresourceRange.baseMipLevel = 0;
        imageviewCI.subresourceRange.levelCount = 1;

        checkVkResult(vkCreateImageView(_vulkanContext.device, &imageviewCI, nullptr, &_renderTarget.vkImageView));
    }
        
    void SandboxLayer::destroyRenderTarget()
    {
        vkDestroyImageView(_vulkanContext.device, _renderTarget.vkImageView, nullptr);
        vmaDestroyImage(_vulkanContext.vmaAllocator, _renderTarget.vkImage, _renderTarget.vmaAllocation);
        _renderTarget = {};
    }

    void SandboxLayer::createDepthTexture()
    {
        VkFormat vkFormat = _vulkanContext.findSupportedFormat({VK_FORMAT_D24_UNORM_S8_UINT}, 
                                                VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        _depthTexture.vkFormat = vkFormat;

        VkImageCreateInfo imageCI = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        imageCI.format = vkFormat;
        imageCI.extent.width = _swapchain.imageExtent.width;
        imageCI.extent.height = _swapchain.imageExtent.height;
        imageCI.extent.depth = 1;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo vmaAllocationCI = {};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        checkVkResult(vmaCreateImage(_vulkanContext.vmaAllocator, &imageCI, &vmaAllocationCI, &_depthTexture.vkImage, &_depthTexture.vmaAllocation, nullptr));

        VkImageViewCreateInfo imageviewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageviewCI.image = _depthTexture.vkImage;
        imageviewCI.format = vkFormat;
        imageviewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageviewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        imageviewCI.subresourceRange.baseArrayLayer = 0;
        imageviewCI.subresourceRange.layerCount = imageCI.arrayLayers;
        imageviewCI.subresourceRange.baseMipLevel = 0;
        imageviewCI.subresourceRange.levelCount = imageCI.mipLevels;

        checkVkResult(vkCreateImageView(_vulkanContext.device, &imageviewCI, nullptr, &_depthTexture.vkImageView));
    }

    void SandboxLayer::destroyDepthTexture()
    {
        vkDestroyImageView(_vulkanContext.device, _depthTexture.vkImageView, nullptr);
        vmaDestroyImage(_vulkanContext.vmaAllocator, _depthTexture.vkImage, _depthTexture.vmaAllocation);
        _depthTexture = {};
    }

    void SandboxLayer::createRenderPass()
    {
        std::array<VkAttachmentDescription, 2> attachmentDesc = {};
        // color attachment
        attachmentDesc[0].format = _swapchain.imageFormat;
        attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // depth attachment
        attachmentDesc[1].format = _depthTexture.vkFormat;
        attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorAttachmentRef;
        subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
        VkRenderPassCreateInfo renderPassCI = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        renderPassCI.attachmentCount = static_cast<uint32_t>(attachmentDesc.size());
        renderPassCI.pAttachments = attachmentDesc.data();
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDesc;
        renderPassCI.dependencyCount = 1;
        renderPassCI.pDependencies = &subpassDependency;

        checkVkResult(vkCreateRenderPass(_vulkanContext.device, &renderPassCI, nullptr, &_renderpass));
    }

    void SandboxLayer::createFramebuffers()
    {
        _frameBuffers.resize(_swapchain.imageCount);

        for (uint32_t i = 0; i != _swapchain.imageCount; ++i) {
            std::array<VkImageView, 2> imageViews = {_swapchain.imageViews[i], _depthTexture.vkImageView};

            VkFramebufferCreateInfo framebufferCI = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            framebufferCI.attachmentCount = imageViews.size();
            framebufferCI.pAttachments = imageViews.data();
            framebufferCI.width = _swapchain.imageExtent.width;
            framebufferCI.height = _swapchain.imageExtent.height;
            framebufferCI.layers = 1;
            framebufferCI.renderPass = _renderpass;

            checkVkResult(vkCreateFramebuffer(_vulkanContext.device, &framebufferCI, nullptr, &_frameBuffers[i]));
        }
    }

    void SandboxLayer::destroyFramebuffers()
    {
        for (const VkFramebuffer framebuffer : _frameBuffers) {
            vkDestroyFramebuffer(_vulkanContext.device, framebuffer, nullptr);
        }
    }

    void SandboxLayer::recreateSwapchain(const VkExtent2D desiredExtent2D)
    {
        checkVkResult(vkDeviceWaitIdle(_vulkanContext.device));

        destroyFramebuffers();
        destroyDepthTexture();
        destroySwapchain(_vulkanContext.device, _swapchain);

        _swapchain = createSwapchain(_vulkanContext.device, _vulkanContext.surface, _vulkanContext.physicalDevice.vkPhysicalDevice, desiredExtent2D);
        createDepthTexture();
        createFramebuffers();
    }

    void SandboxLayer::setupImGui()
    {
        constexpr std::array<VkDescriptorPoolSize, 11> poolSizes = {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo descriptorPoolCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolCI.pPoolSizes = poolSizes.data();
        descriptorPoolCI.maxSets = 1000 * poolSizes.size();

        checkVkResult(vkCreateDescriptorPool(_vulkanContext.device, &descriptorPoolCI, nullptr, &_imguiDescriptorPool));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(Application::getWindow().getNativeWindowHandle(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = _vulkanContext.instance.vkinstance;
        init_info.PhysicalDevice = _vulkanContext.physicalDevice.vkPhysicalDevice;
        init_info.Device = _vulkanContext.device;
        init_info.QueueFamily = _vulkanContext.physicalDevice.queueFamilies.graphicsFamily.value();
        init_info.Queue = _vulkanContext.graphicsQueue;
        init_info.DescriptorPool = _imguiDescriptorPool;
        init_info.MinImageCount = _swapchain.minImageCount;
        init_info.ImageCount = _swapchain.imageCount;
        init_info.CheckVkResultFn = [](VkResult result) { checkVkResult(result); };
        ImGui_ImplVulkan_Init(&init_info, _renderpass);

        Frame &frame = getCurrentFrame();

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.commandPool = frame.commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        checkVkResult(vkAllocateCommandBuffers(_vulkanContext.device, &commandBufferAllocateInfo, &commandBuffer));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        checkVkResult(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

        checkVkResult(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
    
        checkVkResult(vkResetFences(_vulkanContext.device, 1, &frame.frameInFlightFence));
        checkVkResult(vkQueueSubmit(_vulkanContext.graphicsQueue, 1, &submitInfo, frame.frameInFlightFence));
        checkVkResult(vkWaitForFences(_vulkanContext.device, 1, &frame.frameInFlightFence, VK_TRUE, std::numeric_limits<uint64_t>::max()));
    }
}