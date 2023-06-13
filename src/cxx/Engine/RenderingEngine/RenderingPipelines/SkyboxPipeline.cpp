#include "SkyboxPipeline.h"

using namespace RenderEngine;

SkyboxPipeline::SkyboxPipeline(VulkanDevice *device, int width, int height) : RenderPipeline(device, nullptr), device(device)
{
    RenderPipelineBuilder builder;
    builder.addVertexInput(0, 3, sizeof(float), VK_FORMAT_R32G32B32_SFLOAT)
        ->addPushConstant(sizeof(SkyboxConfig), VK_SHADER_STAGE_VERTEX_BIT)
        ->setPathToShader("shaders/SkyboxPipeline")
        ->setStartFramebufferWidth(width)
        ->setStartFramebufferHeight(height)
        ->setImageRenderOutputAmount(1)
        ->addSampler(0, VK_SHADER_STAGE_FRAGMENT_BIT);
    RenderPipeline::initialize(builder);
    RenderPipeline::getPushConstants()[0]->setData(&skyboxConfig);
    descriptorSet = RenderPipeline::acquireDescriptorSet();
    descriptorSet->attachToObject(this);
    skyboxMesh = acquireSkyboxMesh(device);
}

void SkyboxPipeline::update(glm::mat4 cameraMatrix)
{
    skyboxConfig.cameraMatrix = cameraMatrix;
    VkCommandBuffer cmd = RenderPipeline::beginRender();
    RenderPipeline::updatePushConstants();
    descriptorSet->bind(0, cmd, RenderPipeline::getPipelineLayout());
    skyboxMesh->draw(cmd);
    RenderPipeline::endRenderPass();
    RenderPipeline::endRender();
}

void SkyboxPipeline::setSkyboxImage(VulkanCubemapImage *skyboxImage)
{
    descriptorSet->getSamplers()[0]->setSamplerImageView(skyboxImage->getImageView());
    descriptorSet->updateDescriptorSet(0);
}