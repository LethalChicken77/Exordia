#pragma once
#include "asset.hpp"
#include "game_object.hpp"

namespace core
{
// Active scene representation in memory, loaded from SceneAsset
class Scene
{
    public:
        static constexpr const char* className = "Scene";
        const char* GetClassName() const { return className; }

        inline Scene() {loadScene();};
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        void loadScene();
        void update(double deltaTime);

        std::vector<GameObject> &getGameObjects() { return gameObjects; }

        void drawScene();

        std::vector<glm::mat4> transforms{};

        int selectedObject = -1;
        
    protected:
        std::vector<GameObject> gameObjects{};
};

} // namespace core