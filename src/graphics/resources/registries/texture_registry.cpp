#include "texture_registry.hpp"

namespace graphics
{

void TextureRegistry::Init()
{
    std::unique_ptr<TextureData> debugTexture = TextureData::LoadFromFile("./internal/debug/missing_texture.png");
    debugTexture->properties.magFilter = TextureFilterMode::Nearest;
    debugTexture->properties.minFilter = TextureFilterMode::Nearest;
    if(debugTexture == nullptr)
    {
        throw std::runtime_error("Failed to load error texture!");
    }
    Register(*debugTexture);

    debugTextureHandle = debugTexture->graphicsHandle;
}

/// @brief Create graphics mesh data for a given mesh
/// @param texture 
/// @return Handle for new mesh, invalid on failure
TextureHandle TextureRegistry::Register(TextureData &texture)
{
    if(IsValid(texture.graphicsHandle)) return texture.graphicsHandle; // Mesh is already registered

    uint32_t index;
    if(freeList.size() > 0)
    {
        // Take oldest to reduce fragmentation
        index = freeList[0];
        freeList.erase(freeList.begin());
    }
    else
    {
        index = entries.size();
        entries.emplace_back();
    }

    Entry &entry = entries[index];
    entry.value = std::make_unique<Texture>(texture);
    entry.inUse = true;
    entry.generation = nextGeneration++;

    texture.graphicsHandle = {index, entry.generation};
    return texture.graphicsHandle;
}

/// @brief Update the graphics mesh corresponding to an already registered mesh
/// @param texture 
/// @return True on success, false on failure
bool TextureRegistry::Update(TextureData &texture)
{
    TextureHandle &handle = texture.graphicsHandle;
    if(!handle.IsValid()) return false;

    Entry &entry = entries[handle.index];

    // entry.value->SetData(&texture);

    return true;
}

/// @brief Get a pointer to a stored resource
/// @param handle The handle of the resource to retrieve
/// @return Pointer to the resource. nullptr if invalid.
Texture* TextureRegistry::Get(TextureHandle handle) const 
{
    if(!IsValid(handle) && handle == debugTextureHandle)
    {
        return nullptr;
    }
    else if(!IsValid(handle))
    {
        return Get(debugTextureHandle);
    }
    return entries[handle.index].value.get();
};

} // namespace graphics