#pragma once
#include <string>
#include <optional>
#include "texture_data.hpp"

namespace graphics
{

class Sprite
{
public:
    std::string name;
    void SetTexture(TextureData* textureData);

    /// @brief Set the lower left corner of the sprite in pixels
    void SetPosition(int x, int y);
    /// @brief Set the lower left corner of the sprite in pixels
    inline void SetPosition(glm::ivec2 pos)
    {
        SetPosition(pos.x, pos.y);
    };

    /// @brief Set the width and height of the sprite in pixels
    void SetSize(int w, int h);
    /// @brief Set the width and height of the sprite in pixels
    inline void SetSize(glm::ivec2 pos)
    {
        SetSize(pos.x, pos.y);
    };

    /// @brief Set the pivot as a fraction of the texture size. Can extend outside of texture bounds.
    /// @param pos 
    inline void SetPivot(glm::vec2 pos)
    {
        pivot = pos;
    }


    void Validate();

    void SetLeft(int l);
    void SetRight(int r);
    void SetTop(int t);
    void SetBottom(int b);

    [[nodiscard]] inline TextureHandle GetTexture() const { return texture; };
    [[nodiscard]] inline glm::ivec2 GetTextureSize() const { return textureSize; };
    [[nodiscard]] inline float GetTexturePPU() const { return texturePPU; };

    [[nodiscard]] inline glm::ivec2 GetPos() const { return pos; }
    [[nodiscard]] inline glm::ivec2 GetSize() const { return size; }
    [[nodiscard]] inline glm::vec2 GetPivot() const { return pivot; }
    
    [[nodiscard]] inline int GetLeft() const { return left; }
    [[nodiscard]] inline int GetRight() const { return right; }
    [[nodiscard]] inline int GetTop() const { return top; }
    [[nodiscard]] inline int GetBottom() const { return bottom; }
    
    [[nodiscard]] uint32_t Get9SliceQuadCount() const;
    [[nodiscard]] uint32_t GetTiledQuadCount(glm::vec2 size) const;
private:
    TextureHandle texture;
    glm::ivec2 textureSize{};
    float texturePPU = 100;
    // Definition
    glm::ivec2 pos = {0,0};
    glm::ivec2 size = {16,16};
    // Border
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    glm::vec2 pivot = {0.5f,0.5f};
};

} // namespace graphics