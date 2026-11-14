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
    void SetTexture(TextureHandle handle);

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


    void Validate();

    void SetLeft(int l);
    void SetRight(int r);
    void SetTop(int t);
    void SetBottom(int b);
private:
    TextureHandle texture;
    int textureWidth;
    int textureHeight;
    // Definition
    glm::ivec2 pos;
    glm::ivec2 size;
    // Border
    int left;
    int right;
    int top;
    int bottom;

    glm::vec2 pivot;
};

} // namespace graphics