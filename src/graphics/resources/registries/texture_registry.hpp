#pragma once
#include "registry.hpp"
#include "graphics/api/resources/texture_data.hpp"
#include "graphics/resources/texture_resource.hpp"

namespace graphics
{
    
class TextureRegistry : public GraphicsRegistry<TextureResource, TextureHandle>
{
public:
    void Init();
    TextureHandle Register(Texture texture);
    bool Update(Texture texture);
    using GraphicsRegistry<TextureResource, TextureHandle>::Deregister;
    inline bool Deregister(Texture texture)
    {
        if(texture == nullptr) return false;
        return Deregister(texture->graphicsHandle);
    }

    [[nodiscard]] TextureResource* Get(TextureHandle handle) const;
private:
    TextureHandle debugTextureHandle;
};

} // namespace graphics