#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "modules.hpp"
#include "core/input.hpp"
#include "graphics/resources/graphics_mesh.hpp"

#include "graphics/api/resources/shader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "utils/imgui_styles.hpp"
#include "arena_allocator.hpp"
#include "pool_allocator.hpp"

using namespace graphics;

namespace core
{

// struct ObjectUbo
// {

// };

Engine::Engine()
{
    gameData = std::make_unique<GameData>();
    init();
}

Engine::~Engine()
{
    gameData.reset();
    close();
}

void Engine::init()
{
    gameData->skyboxMesh = Mesh::createSkybox(10);
    graphicsModule.RegisterMesh(*gameData->skyboxMesh);

    ShaderAsset *testShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/basicShader.slang");
    ShaderAsset *pbrShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/PBR.slang");
    ShaderAsset *skyboxShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/skybox.slang");

    testShader->LoadData();
    pbrShader->LoadData();
    skyboxShader->LoadData();

    // Shader shader = Shader(testShader, testShader);
    // This is just to test memory pools, this will be replaced with a proper resource management system
    Shader* shader = gameData->shaderPool.New(testShader, testShader);
    Shader* sbShader = gameData->shaderPool.New(skyboxShader, skyboxShader);
    sbShader->properties.depthWrite = DepthWrite::DISABLED;
    // Shader* pbr = shaderPool.New(pbrShader, pbrShader);
    graphicsModule.RegisterShader(*shader);
    graphicsModule.RegisterShader(*sbShader);

    gameData->materials.emplace_back(Material(shader));
    gameData->materials.emplace_back(Material(shader));
    gameData->materials.emplace_back(Material(shader));
    gameData->materials.emplace_back(Material(sbShader));
    int64_t testVal = 69;
    // mat.SetInt("test", testVal);
    gameData->materials[0].SetVector("color", glm::vec4(1.f, 0.8f, 0.3f, 1.0f));
    gameData->materials[0].SetFloat("roughness", 0.4f);
    gameData->materials[0].SetFloat("metallic", 0.0f);

    gameData->materials[1].SetVector("color", glm::vec4(0.2f, 0.5f, 0.8f, 1.0f));
    gameData->materials[1].SetFloat("roughness", 0.2f);
    gameData->materials[1].SetFloat("metallic", 1.0f);

    gameData->materials[2].SetVector("color", glm::vec4(0.1f, 0.5f, 0.1f, 1.0f));
    gameData->materials[2].SetFloat("roughness", 0.8f);
    gameData->materials[2].SetFloat("metallic", 0.0f);

    // mat2.SetVector("color", glm::vec4(1));
    // mat2.SetFloat("normalMapStrength", 1.0f);
    // mat2.SetTexture("albedoMap", );
    // mat2.SetTexture("roughnessMap", );
    // mat2.SetTexture("metallicMap", );
    // mat2.SetTexture("normalMap", );

    graphicsModule.RegisterMaterial(gameData->materials[0]);
    graphicsModule.RegisterMaterial(gameData->materials[1]);
    graphicsModule.RegisterMaterial(gameData->materials[2]);
    graphicsModule.RegisterMaterial(gameData->materials[3]);

    graphicsModule.GraphicsInitImgui();
    // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;


    Input::initializeKeys();


    camera = Camera(
        {0.01f, 1000.0f, 90.0f},
        false
    );
    // camera = Camera(
    //     {0.01f, 100.0f, 20.0f},
    //     true
    // );
    cameraTransform.setPosition(glm::vec3(0.0f, 2.0f, -5.0f));
    cameraTransform.setPosition(glm::vec3(glm::radians(20.0f), 0.0f, 0.0f));

    scene = Scene("Test");
    scene->loadScene();
}

void Engine::close()
{
    // graphicsModule.cleanup();
}

void Engine::update(double deltaTime)
{
    glm::vec3 forward = cameraTransform.forward();
    forward.y = 0;
    forward = glm::normalize(forward);
    forward *= glm::sign(cameraTransform.up().y);
    glm::vec3 right = cameraTransform.right();
    float movementSpeed = 10.f;
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
        cameraTransform.addPosition(glm::vec3(0, 10.f * deltaTime, 0));
    }
    if(core::Input::getKey(GLFW_KEY_LEFT_SHIFT))
    {
        cameraTransform.addPosition(glm::vec3(0, -10.f * deltaTime, 0));
    }

    // TODO: Reimplement object selection
    // if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    // {
    //     glm::vec2 mousePos = core::Input::getMousePosition();
    //     int mouseXPos = mousePos.x;
    //     int mouseYPos = mousePos.y;
    //     Console::debug(std::to_string(mouseXPos) + " " + std::to_string(mouseYPos));
    //     Console::debug(std::to_string(graphicsModule.getClickedObjID(mouseXPos, mouseYPos)));
    //     scene->selectedObject = graphicsModule.getClickedObjID(mouseXPos, mouseYPos);
    // }

    if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphicsModule.GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
        glfwSetInputMode(graphicsModule.GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    scene->update(deltaTime);
}

void Engine::run()
{
    
    // camera.transform.parent = &scene->getGameObjects()[0]->transform;

    // graphicsModule.SetCamera(&camera);

    VkDescriptorSet viewPortDS = nullptr;

    std::cout << "Entering main loop" << std::endl;
    double time = 0.0f;
    double deltaTime = 0.0f;
    // Main loop
    while (graphicsModule.IsOpen()) {
        // Poll for and process events
        glfwPollEvents();
        
        // TODO: Reimplement
        // if(core::Input::getKeyDown(GLFW_KEY_R))
        // {
        //     graphicsModule.ReloadShaders();
        // }


        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        // ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
        // ImVec2 size = ImGui::GetContentRegionAvail();
        // graphicsModule.viewportSize = VkExtent2D{(uint32_t)size.x, (uint32_t)size.y};
        // graphicsModule.updateExtent();
        // viewPortDS = graphicsModule.getViewportDescriptorSet();
        // if(viewPortDS != nullptr)
        // {
        //     ImGui::Image((void*)reinterpret_cast<uintptr_t>(viewPortDS), size);
        // }
        // ImGui::End();
        // ImGui::PopStyleVar(2);

        Console::drawImGui();
        ObjectManager::drawImGui();

        // ImGui::Begin("Material Properties");

        // for(Material &mat : Shared::materials)
        // {
        //     mat.drawImGui();
        // }
        
        // ImGui::End();

        bool imguiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        ImGui::Render();
        // Input
        Input::processInput(graphicsModule.GetGLFWWindow());
        
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
        graphicsModule.DrawMesh(gameData->skyboxMesh, gameData->materials[3], skyboxTransform.getTransform(), -1); // Draw skybox
        scene->drawScene();
        // Render here
        camera.setAspectRatio(graphicsModule.GetAspectRatio());
        camera.SetTransform(cameraTransform.getTransform());
        graphicsModule.SetCamera(camera);
        graphicsModule.DrawFrame();


        // std::cout << "Delta time: " << deltaTime << std::endl;
    }
    close();
}

} // namespace core