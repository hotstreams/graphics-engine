#pragma once

namespace GraphicsEngine {
    enum class MaterialShader {
        Default,
        Deferred,
        ForwardPlus,
        DirectionalShadow,
        PointShadow,
        ColorPicker
    };

    enum class ModelShader {
        Model,
        Skeletal,
        Instanced,
        SkeletalInstanced,
    };
}