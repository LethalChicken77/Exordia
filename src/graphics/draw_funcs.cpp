#include "draw_funcs.hpp"
#include "internal/graphics_pipeline.hpp"

namespace graphics
{

void DrawFunctions::bindCameraDescriptor(RenderContext& frameInfo, GraphicsPipeline* pipeline)
{
    std::vector<VkDescriptorSet> descriptorSets = { frameInfo.cameraDescriptorSet };

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline->getPipelineLayout(), 
        0,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );
}

void DrawFunctions::bindGlobalDescriptor(RenderContext& frameInfo, GraphicsPipeline* pipeline)
{

    std::vector<VkDescriptorSet> descriptorSets = { frameInfo.globalDescriptorSet };

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline->getPipelineLayout(), 
        1,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );
}

void DrawFunctions::renderMeshes(RenderContext& context, const std::vector<MeshRenderData> &renderQueue)
{
    VkCommandBuffer& commandBuffer = context.commandBuffer;

    // pipelineManager.renderObjects(frameInfo, gameObjects, commandBuffer);
    GraphicsPipeline* prevPipeline = nullptr;
    std::vector<VkDescriptorSet> localDescriptorSets;
    VkPipelineLayout pipelineLayout = nullptr;
    Material::id_t prevMaterial = UINT64_MAX;
    for(const MeshRenderData &renderData : renderQueue)
    {
        const Shader* shader = Shared::materials[renderData.materialIndex].getShader();
        GraphicsPipeline* pipeline = shader->getPipeline();
        uint32_t setIndex = pipeline->getID() + 1;
        if(pipeline != prevPipeline) // Bind camera and global data
        {
            pipeline->bind(context.commandBuffer);
            pipelineLayout = pipeline->getPipelineLayout();
            bindCameraDescriptor(context, pipeline);
            bindGlobalDescriptor(context, pipeline);
            prevPipeline = pipeline;
        }
        localDescriptorSets = { Shared::materials[renderData.materialIndex].getDescriptorSet() };

        if(prevMaterial != renderData.materialIndex) // Bind material info if changed
        {
            vkCmdBindDescriptorSets(
                commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelineLayout, 
                2,
                1,
                localDescriptorSets.data(), 
                0,
                nullptr
            );
            prevMaterial = renderData.materialIndex;
        }

        PushConstants push{}; // TODO: Instance specific data
        push.objectID = renderData.meshID; // TODO: Change to scene local ID
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        std::unique_ptr<GraphicsMesh> &graphicsMesh = Shared::graphicsMeshes[renderData.meshID];
        if(graphicsMesh != nullptr)
        {
            graphicsMesh->bind(commandBuffer, renderData.instanceBuffer);
            graphicsMesh->draw(commandBuffer, renderData.transforms.size());
        }
    }
}

void DrawFunctions::renderGameObjectIDs(RenderContext& context, const std::vector<MeshRenderData> &renderQueue)
{
    VkCommandBuffer& commandBuffer = context.commandBuffer;

    // pipelineManager.renderObjects(frameInfo, gameObjects, commandBuffer);
    std::vector<VkDescriptorSet> localDescriptorSets;
    const Shader* shader = Shared::shaders[3].get(); // TODO: Do something better than this
    GraphicsPipeline* pipeline = shader->getPipeline();
    VkPipelineLayout pipelineLayout = pipeline->getPipelineLayout();
    uint32_t setIndex = pipeline->getID() + 1;

    pipeline->bind(context.commandBuffer);
    bindCameraDescriptor(context, pipeline);


    for(const MeshRenderData &renderData : renderQueue)
    {
        PushConstants push{};

        push.objectID = renderData.objectID;
        // Console::debug(std::to_string(push.objectID), "Graphics");
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        std::unique_ptr<GraphicsMesh> &graphicsMesh = Shared::graphicsMeshes[renderData.meshID];
        if(graphicsMesh != nullptr)
        {
            graphicsMesh->bind(commandBuffer, renderData.instanceBuffer);
            graphicsMesh->draw(commandBuffer, renderData.transforms.size());
        }
    }
}

}