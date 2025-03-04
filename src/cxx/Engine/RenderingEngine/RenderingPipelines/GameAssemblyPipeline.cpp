//
// Created by kgaft on 6/7/23.
//

#include "GameAssemblyPipeline.h"

RenderEngine::GameAssemblyPipeline::GameAssemblyPipeline(VulkanDevice *device, int width, int height) : RenderPipeline(device, nullptr), device(device){
    RenderPipelineBuilder builder;
    builder.setPathToShader("shaders/GameAssemblyPipeline")
            ->setStartFramebufferWidth(width)
            ->setStartFramebufferHeight(height)
            ->addVertexInput(0, 3, sizeof(float), VK_FORMAT_R32G32B32_SFLOAT)
            ->addVertexInput(1, 2, sizeof(float), VK_FORMAT_R32G32_SFLOAT);
    for(int i = 1; i<8; i++){
        builder.addSampler(i, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    builder.addPushConstant(sizeof(VertexConfig), VK_SHADER_STAGE_VERTEX_BIT);
    builder.addUniformBuffer(0, sizeof(LightConfiguration), VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setImageRenderOutputAmount(1);
    RenderPipeline::initialize(builder);
    gBufferDescriptor = RenderPipeline::acquireDescriptorSet();
    quadDrawMesh = acquireQuadMesh(device);
    RenderPipeline::getPushConstants()[0]->setData(&vertexConfig);
}

void RenderEngine::GameAssemblyPipeline::setGBuffer(RenderEngine::GBufferPipeline *gBufferPipeline) {
    gBufferDescriptor->attachToObject(gBufferPipeline);
    gBufferDescriptor->getSamplers()[0]->setSamplerImageView(gBufferPipeline->getPositionsImage()->getView());
    gBufferDescriptor->getSamplers()[1]->setSamplerImageView(gBufferPipeline->getAlbedoMapImage()->getView());
    gBufferDescriptor->getSamplers()[2]->setSamplerImageView(gBufferPipeline->getNormalMapImage()->getView());
    gBufferDescriptor->getSamplers()[3]->setSamplerImageView(
            gBufferPipeline->getMetallicRoughnessEmissiveInvao()->getView());
    gBufferDescriptor->getSamplers()[4]->setSamplerImageView(gBufferPipeline->getAoImage()->getView());
    
}

void RenderEngine::GameAssemblyPipeline::setAo(VulkanImage *ao)
{
    gBufferDescriptor->getSamplers()[4]->setSamplerImageView(ao->getView());
}

void RenderEngine::GameAssemblyPipeline::setBackground(VulkanImage *background)
{
    gBufferDescriptor->getSamplers()[6]->setSamplerImageView(background->getView());
}

void RenderEngine::GameAssemblyPipeline::setReflectionImage(VulkanCubemapImage *reflection)
{
    gBufferDescriptor->getSamplers()[5]->setSamplerImageView(reflection->getImageView());
}

RenderEngine::LightConfiguration &RenderEngine::GameAssemblyPipeline::getLightConfig()  {
    return lightConfig;
}

RenderEngine::VertexConfig &RenderEngine::GameAssemblyPipeline::getVertexConfig() {
    return vertexConfig;
}

void RenderEngine::GameAssemblyPipeline::update() {
    VkCommandBuffer cmd = RenderPipeline::beginRender();
    RenderPipeline::getPushConstants()[0]->setData(&vertexConfig);
    gBufferDescriptor->getUniformBuffers()[0]->write(&lightConfig);
    gBufferDescriptor->bind(0, cmd, RenderPipeline::getPipelineLayout());
    quadDrawMesh->draw(cmd);
    RenderPipeline::endRenderPass();
    RenderPipeline::endRender();
}

VulkanImage *RenderEngine::GameAssemblyPipeline::getOutput()
{
    return RenderPipeline::getOutputImages()[0];
}

void RenderEngine::GameAssemblyPipeline::confirmInputs()
{
    gBufferDescriptor->updateDescriptorSet(0);
}
