#pragma once

#include <Arsenic/Arsenic.hpp>

namespace arsenic
{
    constexpr std::size_t maxRenderObjects = 1E6;
    constexpr std::size_t maxMaterials = 1E6;
    constexpr std::size_t maxLights = 1E6;

    class SandboxLayer : public Layer
    {
    public:
        SandboxLayer();
        ~SandboxLayer();

        SandboxLayer(const SandboxLayer &) = delete;
        SandboxLayer &operator=(const SandboxLayer &) = delete;
        SandboxLayer(SandboxLayer &&) = delete;
        SandboxLayer &operator=(SandboxLayer &&) = delete;

        void onUpdate(const float dt) override;
        void onRender() override;
        void onImGuiRender() override;
        void onEvent(Event &e) override;
        void onFrameBegin() override;
        void onFrameEnd() override;
    private:
        void setupShaderResource();
        void createEngineDescriptorPool();
        void setupGlobalDescriptorSet();
        void setupPerPassDescriptorSet();
        void initializeFrame();
        void deInitializeFrame();
        void createRenderTarget();
        void destroyRenderTarget();
        void createDepthTexture();
        void destroyDepthTexture();
        void createRenderPass();
        void createFramebuffers();
        void destroyFramebuffers();
        void recreateSwapchain(const VkExtent2D desiredExtent2D);
        Frame &getCurrentFrame() { return _frames.value[_currentFrame]; }
        const Frame &getCurrentFrame() const { return _frames.value[_currentFrame]; }
        void setupImGui();
    private:
        VulkanContext _vulkanContext;
        MaterialManager _materialManager;   

        PerFrame<Frame> _frames;
        uint32_t _currentFrame = 0;
        Swapchain _swapchain;
        VulkanImage _depthTexture;
        std::vector<VkFramebuffer> _frameBuffers;
        VkRenderPass _renderpass;

        VulkanImage _renderTarget;
        VkExtent2D _renderTargetExtent;

        VkDescriptorPool _imguiDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

        PerFrame<VkDescriptorSet> _globalDescriptorSet;
        PerFrame<VkDescriptorSet> _perPassDescriptorSets;

        PerFrame<VulkanBuffer> _gpuSceneBuffers;
        PerFrame<VulkanBuffer> _gpuCameraBuffers;
        PerFrame<VulkanBuffer> _gpuRenderObjectBuffers;
        PerFrame<VulkanBuffer> _gpuLightBuffers;
        PerFrame<VulkanBuffer> _gpuMaterialBuffers;

        SceneBuffer _sceneBuffer;
        CameraBuffer _cameraBuffer;
        std::vector<MeshObject> _meshObjects;
        std::vector<RenderObject> _renderObjects;
        std::vector<Light> _lights;
        std::vector<Material> _materials;
        
        VulkanImage _sceneEnviromentMap;
        VkSampler _generalSampler;
        
        ShaderEffect _rtShaderEffect;
        ShaderEffect _fullScreenShaderEffect;
        ShaderPass _rtShaderPass;
        ShaderPass _fullScreenPass;
     
        Scene _scene;
        Camera _camera;

        Entity _cubeEntity;
        Entity _dirLightEntity;
    };
} 