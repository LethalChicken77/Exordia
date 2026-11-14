cmake_minimum_required(VERSION 3.20)
project(GameEngine LANGUAGES CXX C)

# Compiler & basic settings
# set(CMAKE_C_COMPILER "C:/msys64/clang64/bin/clang.exe") # Set to your C compiler path
# set(CMAKE_CXX_COMPILER "C:/msys64/clang64/bin/clang++.exe") # Set to your C++ compiler path
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# set(CMAKE_BUILD_TYPE Release) # Options: Debug, Release, RelWithDebInfo, MinSizeRel

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # YOLO
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
endif()

set(SANITIZE OFF)
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    add_compile_options(
        /diagnostics:column
        /clang:-fno-diagnostics-show-option
        /clang:-fno-diagnostics-show-note-include-stack
        /clang:-fno-color-diagnostics
        /clang:-fdiagnostics-absolute-paths
        /clang:-fno-ms-compatibility
    )
else()
    add_compile_options(
        -fdiagnostics-format=msvc
        -fno-caret-diagnostics
        -fno-diagnostics-show-option
        -fno-diagnostics-show-note-include-stack
        -fno-color-diagnostics
        -fdiagnostics-absolute-paths
    # -fmacro-prefix-map=${CMAKE_SOURCE_DIR}/=.
    # -ffile-prefix-map=${CMAKE_SOURCE_DIR}/=.
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options(
            /Od
            /Zi
            /clang:-fno-omit-frame-pointer
        )
    else()
        add_compile_options(
            -g
            -fno-omit-frame-pointer
        )
    endif()

    if(SANITIZE)
        if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            add_compile_options(/fsanitize=address)
        else()
            add_compile_options(-fsanitize=address)
        endif()
    endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        add_compile_options(
            /O2
            /clang:-march=native
            /fp:fast
        )
    else()
        add_compile_options(
            -O3
            -march=native
            -ffast-math
            -fopenmp-simd
        )
    endif()
endif()
if(WIN32)
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/lib/miniz-3.0.2/miniz.c"
        PROPERTIES COMPILE_DEFINITIONS "_CRT_DECLARE_NONSTDC_NAMES=1"
    )
endif()
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${ASAN_LIB_PATH} -lclang_rt.asan_dynamic -lclang_rt.asan_dynamic_runtime_thunk")


# Specify the output directory for the built binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_AUTORCC OFF)

# Vulkan + Volk
# Use the environment variable that the SDK always sets
set(VULKAN_SDK "$ENV{VULKAN_SDK}")
cmake_path(SET VULKAN_SDK NORMALIZE "${VULKAN_SDK}")
message(STATUS "Vulkan SDK Path: ${VULKAN_SDK}")
# Set GLFW Include Directory
set(GLFW_INCLUDE_DIR "C:/glfw-3.4.bin.WIN64/include")
include_directories(${GLFW_INCLUDE_DIR})

# Include Vulkan headers
# include_directories(${Vulkan_INCLUDE_DIRS})

# Include directories for your project
include_directories("include")
include_directories("lib/tinyobjloader")
include_directories("lib/miniz-3.0.2")
include_directories("lib/tinyexr-1.0.12")
include_directories("lib/stbimage")
include_directories("lib/imgui")
include_directories("lib/nativefiledialog-release_116/src/include")


# add_library(slang SHARED IMPORTED)
# set_target_properties(slang PROPERTIES
#     IMPORTED_LOCATION "${Vulkan_SDK_ROOT}/Bin/slang.dll"
#     INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_SDK_ROOT}/Include/slang"
# )

# Add ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS}/imgui_impl_vulkan.cpp
)

set(VOLK_SOURCE
    ${VULKAN_SDK}/Include/Volk/volk.c
)

# Add all source files from ./src
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
file(GLOB_RECURSE LIB_SOURCES
    "lib/miniz-3.0.2/*.c"
    "lib/nativefiledialog-release_116/src/nfd_common.c"
    "lib/nativefiledialog-release_116/src/nfd_win.cpp"
    "lib/spirv-reflect/spirv_reflect.cpp"
)

# OpenMP setup 
# OpenMP setup
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(OpenMP_C_FLAGS "/openmp:experimental")
    set(OpenMP_CXX_FLAGS "/openmp:experimental")
else()
    set(OpenMP_C_FLAGS "-fopenmp")
    set(OpenMP_CXX_FLAGS "-fopenmp")
endif()


find_package(OpenMP COMPONENTS CXX)
if(OpenMP_CXX_FOUND)
    set(LIBRARIES ${LIBRARIES} OpenMP::OpenMP_CXX)
endif()

# Add executable
add_executable(GameEngine 
    ${SOURCES} 
    ${VOLK_SOURCE} 
    ${IMGUI_SOURCES} 
    ${LIB_SOURCES} 
    app.rc)
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/src/core") # Directly including common directories for shorter includes
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/src/memory")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/src/utils")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/tinyobjloader")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/miniz-3.0.2")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/tinyexr-1.0.12")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/stbimage")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/include")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/spirv-reflect")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/VulkanMemoryAllocator-Hpp")
target_include_directories(GameEngine PRIVATE "${VULKAN_SDK}/Include")
target_include_directories(GameEngine PRIVATE "${VULKAN_SDK}/Include/Volk")
target_include_directories(GameEngine PRIVATE "${VULKAN_SDK}/Include/vulkan") # Needed for vulkan.hpp
target_include_directories(GameEngine PRIVATE SYSTEM "${VULKAN_SDK}/Include/slang")
# target_include_directories(OpenMP::OpenMP_CXX INTERFACE C:/msys64/clang64/include)
# target_include_directories(GameEngine PRIVATE "${Vulkan_INCLUDE_DIRS}/vma")

if(WIN32)
    set_target_properties(GameEngine PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(GameEngine PRIVATE DEBUG)
endif()
target_compile_definitions(GameEngine PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)
target_compile_definitions(GameEngine PRIVATE VK_NO_PROTOTYPES)
# target_compile_definitions(GameEngine PRIVATE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
# target_compile_definitions(GameEngine PRIVATE VOLK_ENFORCE_SUCCESS=1)
target_compile_definitions(GameEngine PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)
target_compile_definitions(GameEngine PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(GameEngine PRIVATE GRAPHICS_TESTS)

# target_compile_definitions(GameEngine PRIVATE VULKAN_HPP_NO_EXCEPTIONS)
target_compile_definitions(GameEngine PRIVATE VULKAN_SDK="${VULKAN_SDK}") # TODO: Only set in editor builds. Might be irrelevant if I make a new CMakeLists for the runtime.

# target_compile_definitions(GameEngine PRIVATE DISABLE_VALIDATION) # Disable validation checks for performance. Ensure correctness manually.

target_compile_options(GameEngine PRIVATE
    -Wno-nullability-completeness    # Silence warnings from VMA
    -Wno-nullability-extension
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(SANITIZE)
    target_link_options(GameEngine PRIVATE
        -fsanitize=address
    )
    endif()
endif()
if(WIN32)
# set_target_properties(GameEngine PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_options(GameEngine PRIVATE /SUBSYSTEM:CONSOLE)
endif()
# target_link_options(GameEngine PRIVATE
#     -static-libstdc++
#     -static-libgcc
# )
# Link libraries
# TODO: Linux version
target_link_libraries(GameEngine PRIVATE 
    "C:/glfw-3.4.bin.WIN64/lib-vc2022/glfw3.lib"    # Link GLFW
    "${VULKAN_SDK}/Lib/vulkan-1.lib"                     # Link Vulkan
    # "${VULKAN_SDK}/Lib/volk.lib"                     # Link Volk
    "${VULKAN_SDK}/Lib/slang.lib"
    gdi32 user32 shell32 Dwmapi
    OpenMP::OpenMP_CXX
)
