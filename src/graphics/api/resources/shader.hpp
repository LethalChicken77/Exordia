// #pragma once
// #include <cstdint>
// #include <slang.h>
// #include "graphics/api/shader_config.hpp"
// #include "graphics/resources/layouts/shader_layout.hpp"
// #include "graphics/resources/layouts/vertex_layout.hpp"
// #include "graphics/api/handles.hpp"
// #include "graphics/api/assets/shader_asset.hpp"

// #include "glm/glm.hpp"
// #include "primitives/color.hpp"
// // #define __STDCPP_FLOAT16_T__
// // #include <stdfloat>

// namespace graphics
// {
// class Shader
// {
// public:
//     Shader(std::string_view vertAssetPath, std::string_view fragAssetPath);
//     Shader(std::string_view vertAssetPath, std::string_view fragAssetPath, const ShaderProperties& properties);
//     ~Shader();

//     Shader(const Shader&) = delete;
//     Shader& operator=(const Shader&) = delete;

//     void Compile();
//     void Update();

//     const std::vector<uint32_t>& GetVertSpirv() const { return vertSpirv; }
//     const std::vector<uint32_t>& GetFragSpirv() const { return fragSpirv; }
//     const ShaderLayout& GetLayout() const { return layout; }
//     const ShaderLayout* GetLayoutPtr() const { return &layout; }
//     const VertexLayout& GetVertexLayout() const { return vertexLayout; }
//     const VertexLayout& GetInstanceLayout() const { return instanceLayout; }

//     ShaderProperties properties{};

//     ShaderHandle graphicsHandle;

// private:
//     Ref<ShaderCodeAsset> vertexShaderCode{};
//     Ref<ShaderCodeAsset> fragmentShaderCode{};

//     std::vector<uint32_t> vertSpirv{};
//     std::vector<uint32_t> fragSpirv{};
//     ShaderLayout layout{};
//     VertexLayout vertexLayout{};
//     VertexLayout instanceLayout{};
//     bool isValid; // Used to prevent passing bad data to the graphics module if compilation fails

//     void compileFrag();
//     void compileVert();
// };

// class ComputeShader
// {
// public:
//     ComputeShader(ShaderCodeAsset& asset) 
//         : ShaderCodeAsset(&asset) {}
//     ~ComputeShader() = default;

//     ComputeShader(const ComputeShader&) = delete;
//     ComputeShader& operator=(const ComputeShader&) = delete;

//     void Compile();

//     const std::vector<uint32_t>& GetSpirv() const { return spirv; }
//     const graphics::ShaderLayout &GetLayout() const { return layout; }

// private:

//     const ShaderCodeAsset *ShaderCodeAsset;
//     std::vector<uint32_t> spirv;
//     ShaderLayout layout;
// };

// } // namespace graphics