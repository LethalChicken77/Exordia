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

    [[nodiscard]] inline float GetUVLeft() const { return left / (float)textureSize.x; }
    [[nodiscard]] inline float GetUVRight() const { return right / (float)textureSize.x; }
    [[nodiscard]] inline float GetUVTop() const { return top / (float)textureSize.y; }
    [[nodiscard]] inline float GetUVBottom() const { return bottom / (float)textureSize.y; }
    [[nodiscard]] inline glm::vec4 GetUVBorder() const { return {GetUVLeft(), GetUVRight(), GetUVTop(), GetUVBottom()}; }
    
    [[nodiscard]] inline glm::vec2 GetUVMin() const { return (glm::vec2)pos / (glm::vec2)textureSize; }
    [[nodiscard]] inline glm::vec2 GetUVMax() const { return (glm::vec2)(pos + size) / (glm::vec2)textureSize; }
    
    /// @brief Get the number of quads needed to render the sprite as a 9-slice. 
    [[nodiscard]] uint32_t Get9SliceQuadCount() const;
    /// @brief Get the number of quads needed to render the sprite as a tiled 9-slice. 
    [[nodiscard]] uint32_t GetTiledQuadCount(glm::vec2 size) const;

    /// @brief Get size of the sprite based on texture PPU and rect
    [[nodiscard]] glm::vec2 GetSimpleSize() const;
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

enum class SpriteDrawMode
{
    Stretch,
    Tile
};

struct SpriteDrawInfo
{
    Sprite sprite{};
    glm::vec2 size{};
    SpriteDrawMode drawMode{};
};

} // namespace graphics