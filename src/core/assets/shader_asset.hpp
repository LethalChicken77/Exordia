#include "core/asset.hpp"
#include "core/asset_data.hpp"

namespace core
{
class ShaderAsset_T : public AssetData
{

};

class Shader
{
    public:
        SmartRef<ShaderAsset_T> vertexShader;
        SmartRef<ShaderAsset_T> fragmentShader;
};

class ComputeShader
{
    public:
        SmartRef<ShaderAsset_T> computeShader;
};
} // namespace core