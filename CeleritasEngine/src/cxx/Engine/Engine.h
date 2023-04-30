//
// Created by KGAFT on 3/17/2023.
//
#pragma once

#include <Vulkan/VulkanInstance.h>
#include "../Window/Window.h"
#include <Vulkan/VulkanDevice/VulkanDevice.h>
#include <Vulkan/VulkanSwapChain.h>
#include <Vulkan/VulkanLogger/DefaultVulkanLoggerCallback.h>
#include <Vulkan/VulkanImage/VulkanImage.h>
#include "../Util/ModelLoader.h"
#include "Pipelines/AssemblyPipeline.h"

#include "Camera/CameraManager.h"
#include "../TestKeyboardCallback.h"
#include "Pipelines/GBufferPipeline.h"


struct EngineDevice {
    std::string name;
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
};

class Engine : public IWindowResizeCallback {
private:
    static inline VulkanInstance *instance = nullptr;
    static inline bool debugBuild = false;

public:
    static void createInstance(const char *appName, bool debugBuild) {
        instance = new VulkanInstance;
        Engine::debugBuild = debugBuild;
        uint32_t extCount;
        const char **exts = glfwGetRequiredInstanceExtensions(&extCount);
        std::vector<const char *> extensions;
        for (int i = 0; i < extCount; ++i) {
            extensions.push_back(exts[i]);
        }

        if (!instance->createInstance(appName, debugBuild, extensions)) {
            throw std::runtime_error("Failed to init Vulkan API");
        }
        if (debugBuild) {
            VulkanLogger::registerCallback(new DefaultVulkanLoggerCallback);
        }
    }

    static std::vector<EngineDevice> enumSupportedDevices(Window *window) {
        if (instance == nullptr) {
            throw std::runtime_error("Error: you need to create instance firstly");
        }
        std::vector<EngineDevice> result;
        for (const auto &item: VulkanDevice::enumerateSupportedDevices(instance->getInstance(),
                                                                       window->getWindowSurface(
                                                                               instance->getInstance()))) {
            result.push_back({item.second.deviceName, item.first, item.second});
        }
        return result;
    }

private:
    VulkanDevice *device;
    Window *window;
    VulkanSwapChain *swapChain;
    VulkanImage *skyPlaceHolder;
    VulkanImage *gamePlaceHolder;
    AssemblyPipeline *asmPipeline;
    GBufferPipeline *gBufferPipeline;

    CameraManager *manager;
    std::vector<Mesh *> meshes;
    std::vector<Mesh *> helmetMeshes;
    Material material;
    long long frameCounter = 0;
public:
    Engine(EngineDevice &deviceToCreate, Window *window) : window(window) {
        if (instance == nullptr) {
            throw std::runtime_error("Error: you need to create instance firstly");
        }
        device = new VulkanDevice(deviceToCreate.device, window->getWindowSurface(instance->getInstance()),
                                  instance->getInstance(), debugBuild);
        swapChain = new VulkanSwapChain(device, window->getWidth(), window->getHeight());
        gBufferPipeline = new GBufferPipeline(device, window->getWidth(), window->getHeight());
        skyPlaceHolder = VulkanImage::loadTexture("models/bluecloud_ft.jpg", device);
        gBufferPipeline->setSkyBox(skyPlaceHolder);
        ModelLoader loader(device);

        meshes = loader.loadModel("models/pokedex/pokedex.gltf", false);
        material.setAlbedoTexture(VulkanImage::loadTexture("models/pokedex/textures/basecolor.tga", device));
        material.setMetallicTexture(VulkanImage::loadTexture("models/pokedex/textures/metallic.tga", device));
        material.setRoughnessTexture(VulkanImage::loadTexture("models/pokedex/textures/roughness.tga", device));
        material.setNormalMap(VulkanImage::loadTexture("models/pokedex/textures/normal.tga", device));
        material.setEmissiveTexture(VulkanImage::loadTexture("models/pokedex/textures/emissive.tga", device));
        //material.setAoTexture(VulkanImage::loadTexture("models/pokedex/textures/ao.tga", device));

        meshes[0]->setMaterial(&material);
        loader.clear();
        helmetMeshes = loader.loadModel("models/Helmet/DamagedHelmet.gltf", true);
        VulkanImage *albedoIm = VulkanImage::loadTexture("models/Helmet/Default_albedo.jpg", device);
        for (const auto &item: helmetMeshes) {
            item->getMaterial()->setAoTexture(nullptr);
            item->getMaterial()->setOpacityMapTexture(nullptr);
            item->getMaterial()->setAlbedoTexture(albedoIm);
            item->setPosition(glm::vec3(0, 0, 5));
            item->setScale(glm::vec3(5, 5, 5));
            meshes.push_back(item);
        }
        for (const auto &item: meshes){
            gBufferPipeline->registerMaterial(item->getMaterial());
        }
        manager = new CameraManager(&gBufferPipeline->getCameraData());
        TestKeyboardCallback *keyBoardCB = new TestKeyboardCallback(Window::getInstance());
        Window::getInstance()->registerKeyCallback(keyBoardCB);

        gamePlaceHolder = VulkanImage::loadTexture("shaders/game.png", device);
        asmPipeline = new AssemblyPipeline(device, swapChain, Window::getInstance()->getWidth(),
                                           Window::getInstance()->getHeight());


        asmPipeline->setGamePlaceHolder(gBufferPipeline->getOutputImages()[0]);
        asmPipeline->setUiPlaceHolder(skyPlaceHolder);
        asmPipeline->getData().mode = 1;
        asmPipeline->confirmPlaceHolders();
        meshes[0]->setPosition(glm::vec3(0.0f, 1.5f, 0.0));

        window->registerResizeCallback(this);
    }

    void update() {
        manager->update();
        gBufferPipeline->beginRender();
        for (const auto &item: meshes){
            gBufferPipeline->processMesh(item);
        }
        gBufferPipeline->endRender();
        asmPipeline->getData().mode = 1;
        asmPipeline->update();
    }

    void resized(int width, int height) override {
        vkDeviceWaitIdle(device->getDevice());
        asmPipeline->resize(width, height);
        asmPipeline->setGamePlaceHolder(gamePlaceHolder);
        asmPipeline->confirmPlaceHolders();
        frameCounter++;
    }
};
