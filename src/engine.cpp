#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "asset.hpp"
#include "asset_manager.hpp"
#include "graphics/api/assets/material_asset.hpp"
#include "graphics/api/resources/material.hpp"
#include "modules.hpp"
#include "core/input.hpp"
#include "graphics/resources/graphics_mesh.hpp"
#include "core/asset_manager.hpp"

#include "graphics/api/assets/shader_asset.hpp"
#include "object_manager.hpp"
#include "primitives/color.hpp"
#include "temp_data.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "utils/imgui_styles.hpp"
#include "arena_allocator.hpp"
#include "pool_allocator.hpp"
#include "analysis/rolling_average.hpp"

#include "console.hpp"

using namespace graphics;

namespace core
{

// struct ObjectUbo
// {

// };

Engine::Engine()
{
    graphicsModule = std::make_unique<Graphics>(APPLICATION_NAME, ENGINE_NAME);

    if(!graphicsModule->IsOpen())
    {
        throw std::runtime_error("Failed to initialize graphics");
    }

    gameData = std::make_unique<GameData>();
    init();
}

Engine::~Engine()
{
    scene.reset();
    gameData.reset();
    ObjectManager::Destroy();
    graphicsModule.reset();
    close();
}

void Engine::init() {
    gameData->skyboxMesh = Mesh::createSkybox(10);
    graphicsModule->RegisterMesh(*gameData->skyboxMesh);

    gameData->yellowMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/basic_yellow.material");
    gameData->blueMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/basic_blue.material");
    gameData->skyboxMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/skybox.material");
    gameData->spriteMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/sprite.material");
    gameData->spriteTiledMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/sprite_tiled.material");
    gameData->groundMaterial = AssetManager::LoadAsset<MaterialAsset>("assets/ground.material");

    Ref<TextureAsset> textureTest = AssetManager::LoadAsset<TextureAsset>("assets/textures/rocky_terrain_02/rocky_terrain_02_diff_512.jpg");
    Ref<TextureAsset> textureTest2 = AssetManager::LoadAsset<TextureAsset>("assets/textures/rocky_terrain_02/rocky_terrain_02_rough_512.png");
    // spriteShader->properties.depthTest = CompareOp::Always;
    // spriteShader->properties.drawMode = DrawMode::Wireframe;
    // Shader* pbr = shaderPool.New(pbrShader, pbrShader);

    // testSprite.SetTexture(texSpriteP);
    // testSprite.SetPosition({6, 96});
    // testSprite.SetSize({20,32});
    // testSprite.name = "upgrade_rupture_core";
    // testSpriteStretch.SetTexture(texSpriteTileP);
    // testSpriteStretch.SetPosition({0,0});
    // testSpriteStretch.SetSize(texSpriteTileP->GetDimensions());
    // testSpriteStretch.SetLeft(8);
    // testSpriteStretch.SetRight(8);
    // testSpriteStretch.SetTop(8);
    // testSpriteStretch.SetBottom(8);
    // testSpriteStretch.name = "environment_wall_stone";


    // spriteMat->SetTexture("mainTex", thSprite);
    // spriteMatStretch->SetTexture("mainTex", thSpriteTile);

    // gameData->skyboxMaterial = skyboxMat;

    graphicsModule->GraphicsInitImgui();
    // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;


    Input::initializeKeys();


    CameraProperties camProps{};
    camProps.near = 0.01f;
    camProps.far = std::numeric_limits<float>::infinity();
    camera = Camera(camProps);
    cameraTransform.setPosition(glm::vec3(0.0f, 2.0f, 5.0f));
    cameraTransform.setRotation(glm::vec3(glm::radians(20.0f), glm::radians(-180.0f), 0.0f));

    // CameraProperties camProps{};
    // camProps.orthographic = true;
    // camProps.near = 0.0001f;
    // camProps.far = 100.f;
    // camProps.vFov = 18.f;
    // camProps.reversedDepth = false;
    // camera = Camera(camProps);
    // cameraTransform.setPosition(glm::vec3(0.0f, 0.0f, -10.0f));
    // cameraTransform.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));

    // camera = Camera(
    //     {0.01f, 100.0f, 20.0f},
    //     true
    // );

    scene = std::make_unique<Scene>();
}

void Engine::close()
{
    // graphicsModule->cleanup();
}

void Engine::update(double deltaTime)
{
    glm::vec3 forward = cameraTransform.forward();
    forward.y = 0;
    forward = glm::normalize(forward);
    forward *= glm::sign(cameraTransform.up().y);
    glm::vec3 right = cameraTransform.right();
    float movementSpeed = 10.f;
    if(core::Input::getKey(GLFW_KEY_LEFT_CONTROL))
    {
        movementSpeed *= 10.f;
    }
    if(core::Input::getKey(GLFW_KEY_A))
    {
        cameraTransform.addPosition(-movementSpeed * (float)deltaTime * right);
    }
    if(core::Input::getKey(GLFW_KEY_D))
    {
        cameraTransform.addPosition(movementSpeed * (float)deltaTime * right);
    }
    if(core::Input::getKey(GLFW_KEY_W))
    {
        cameraTransform.addPosition(movementSpeed * (float)deltaTime * forward);
    }
    if(core::Input::getKey(GLFW_KEY_S))
    {
        cameraTransform.addPosition(-movementSpeed * (float)deltaTime * forward);
    }
    if(core::Input::getKey(GLFW_KEY_SPACE))
    {
        cameraTransform.addPosition(glm::vec3(0, movementSpeed * deltaTime, 0));
    }
    if(core::Input::getKey(GLFW_KEY_LEFT_SHIFT))
    {
        cameraTransform.addPosition(glm::vec3(0, -movementSpeed * deltaTime, 0));
    }

    if(core::Input::getKeyDown(GLFW_KEY_R))
    {
        // TODO: Make a better system for this
        gameData->yellowMaterial->GetShader()->Reload();
        gameData->skyboxMaterial->GetShader()->Reload();
        gameData->spriteMaterial->GetShader()->Reload();
    }

    if(core::Input::getKeyDown(GLFW_KEY_F10))
    {
        graphicsModule->GetWindow()->ToggleFullscreenBorderless();
    }
    if(core::Input::getKeyDown(GLFW_KEY_F11))
    {
        graphicsModule->GetWindow()->ToggleFullscreen();
    }

    // TODO: Reimplement object selection
    // if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    // {
    //     glm::vec2 mousePos = core::Input::getMousePosition();
    //     int mouseXPos = mousePos.x;
    //     int mouseYPos = mousePos.y;
    //     Console::debug(std::to_string(mouseXPos) + " " + std::to_string(mouseYPos));
    //     Console::debug(std::to_string(graphicsModule->getClickedObjID(mouseXPos, mouseYPos)));
    //     scene->selectedObject = graphicsModule->getClickedObjID(mouseXPos, mouseYPos);
    // }

    if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphicsModule->GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if(core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        // std::cout << "Mouse position: " << core::Input::getMousePosition().x << ", " << core::Input::getMousePosition().y << std::endl;
        glm::vec2 mouseDelta = -core::Input::getMouseDelta();
        mouseDelta = glm::pow(glm::abs(mouseDelta), glm::vec2(1.1f)) * glm::vec2(glm::sign(mouseDelta.x), glm::sign(mouseDelta.y));
        cameraTransform.rotateYaw(-mouseDelta.x * 0.016f * 0.1f, false);
        cameraTransform.rotatePitch(-mouseDelta.y * 0.016f * 0.1f, true);
    }
    if(core::Input::getButtonUp(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphicsModule->GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    scene->update(deltaTime);
}

void Engine::run()
{
    // camera.transform.parent = &scene->getGameObjects()[0]->transform;

    // graphicsModule->SetCamera(&camera);

    VkDescriptorSet viewPortDS = nullptr;

    analysis::RollingAverage<float> averageFrameTime{120};
    std::cout << "Entering main loop" << std::endl;
    double time = 0.0f;
    double deltaTime = 0.0f;
    // Main loop
    while (graphicsModule->IsOpen()) {
        // Poll for and process events
        glfwPollEvents();
        
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        // ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
        // ImVec2 size = ImGui::GetContentRegionAvail();
        // graphicsModule->viewportSize = VkExtent2D{(uint32_t)size.x, (uint32_t)size.y};
        // graphicsModule->updateExtent();
        // viewPortDS = graphicsModule->getViewportDescriptorSet();
        // if(viewPortDS != nullptr)
        // {
        //     ImGui::Image((void*)reinterpret_cast<uintptr_t>(viewPortDS), size);
        // }
        // ImGui::End();
        // ImGui::PopStyleVar(2);

        Console::drawImGui();
        // ObjectManager::drawImGui();
        drawPerformancePanel(deltaTime, averageFrameTime.GetAverage());
        averageFrameTime.PushValue(deltaTime);

        ImGui::Begin("Material Properties");

        if(gameData->yellowMaterial && gameData->yellowMaterial->IsReady() && ImGui::CollapsingHeader(gameData->yellowMaterial->GetMaterial()->name.c_str()))
        {
            gameData->yellowMaterial->GetMaterial().DrawImGui();
        }
        if(gameData->blueMaterial && gameData->blueMaterial->IsReady() && ImGui::CollapsingHeader(gameData->blueMaterial->GetMaterial()->name.c_str()))
        {
            gameData->blueMaterial->GetMaterial().DrawImGui();
        }
        // for(Material &mat : Shared::materials)
        // {
        //     mat.drawImGui();
        // }
        
        ImGui::End();


        bool imguiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        ImGui::Render();
        // ImGui::UpdatePlatformWindows();
        // ImGui::RenderPlatformWindowsDefault();
        // Input
        Input::processInput(graphicsModule->GetGLFWWindow());
        
        if(core::Input::getKeyDown(GLFW_KEY_ESCAPE))
        {
            break;
        }

        // Update Time
        double oldTime = time;
        time = glfwGetTime();
        deltaTime = time - oldTime;
        
        update(deltaTime);

        Transform skyboxTransform{};
        skyboxTransform.setPosition(cameraTransform.getPosition());
        graphicsModule->DrawMesh(gameData->skyboxMesh, gameData->skyboxMaterial->GetMaterial(), skyboxTransform.getTransform(), -1); // Draw skybox
        // graphicsModule->DrawSprite({testSpriteStretch, {29,29}, SpriteDrawMode::Tile}, *gameData->materials.Get(5), glm::identity<glm::mat4>(), -1);
        // graphicsModule->DrawSprite(testSprite, *gameData->materials.Get(4), glm::identity<glm::mat4>(), -1);
        scene->drawScene();
        // Render here
        camera.setAspectRatio(graphicsModule->GetAspectRatio());
        camera.SetTransform(cameraTransform.getTransform());
        graphicsModule->SetCamera(camera);
        graphicsModule->DrawFrame();

        // std::cout << "Delta time: " << deltaTime << std::endl;
    }
    close();
}

void Engine::drawPerformancePanel(float deltaTime, float smoothedDeltaTime)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f) // pivot: top-right
    );

    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Once);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove;
    ImGui::Begin("Performance", nullptr, flags);

    float fps = 1/smoothedDeltaTime;
    ImGui::Text("%.1f FPS\n%.2f ms", fps, smoothedDeltaTime * 1000);
    float rawfps = 1/deltaTime;
    ImGui::Text("%.1f FPS (raw)\n%.2f ms (raw)      ", rawfps, deltaTime * 1000);

    ImGui::End();
}

} // namespace core