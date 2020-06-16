#include "BasicTriangleTest.h"

NS_CC_BEGIN

void BasicTriangle::destroy()
{
    CC_SAFE_DESTROY(_vertexBuffer);
    CC_SAFE_DESTROY(_inputAssembler);
    CC_SAFE_DESTROY(_uniformBuffer);
    CC_SAFE_DESTROY(_shader);
    CC_SAFE_DESTROY(_bindingLayout);
    CC_SAFE_DESTROY(_pipelineLayout);
    CC_SAFE_DESTROY(_pipelineState);
    CC_SAFE_DESTROY(_indexBuffer);
    CC_SAFE_DESTROY(_indirectBuffer);
}

bool BasicTriangle::initialize()
{
    createShader();
    createVertexBuffer();
    createInputAssembler();
    createPipeline();

    return true;
}

void BasicTriangle::createShader()
{
    GFXShaderStageList shaderStageList;
    GFXShaderStage vertexShaderStage;
    vertexShaderStage.type = GFXShaderType::VERTEX;
    
//#if (CC_PLATFORM == CC_PLATFORM_MAC_OSX && defined(USE_METAL))
//    vertexShaderStage.source = R"(
//        #include <metal_stdlib>
//        #include <simd/simd.h>
//
//        using namespace metal;
//
//        struct main0_out
//        {
//            float4 gl_Position [[position]];
//        };
//
//        struct main0_in
//        {
//            float2 a_position [[attribute(0)]];
//        };
//
//        vertex main0_out main0(main0_in in [[stage_in]])
//        {
//            main0_out out = {};
//            out.gl_Position = float4(in.a_position, 0.0, 1.0);
//            return out;
//        }
//    )";
//#else
    
#if defined(USE_VULKAN) || defined(USE_METAL)
    vertexShaderStage.source = R"(
        layout(location = 0) in vec2 a_position;
        void main()
        {
            gl_Position = vec4(a_position, 0.0, 1.0);
        }
    )";
#elif defined(USE_GLES2)
    vertexShaderStage.source = R"(
        attribute vec2 a_position;
        void main()
        {
            gl_Position = vec4(a_position, 0.0, 1.0);
        }
    )";
#else
    vertexShaderStage.source = R"(
    in vec2 a_position;
    void main()
    {
    gl_Position = vec4(a_position, 0.0, 1.0);
    }
    )";
#endif // USE_GLES2
    
//#endif // (CC_PLATFORM == CC_PLATFORM_MAC_OSX)
    shaderStageList.emplace_back(std::move(vertexShaderStage));

    GFXShaderStage fragmentShaderStage;
    fragmentShaderStage.type = GFXShaderType::FRAGMENT;
    
//#if (CC_PLATFORM == CC_PLATFORM_MAC_OSX && defined(USE_METAL))
//    fragmentShaderStage.source = R"(
//    #ifdef GL_ES
//    precision highp float;
//    #endif
//    layout(std140, binding = 0) uniform Color
//    {
//        vec4 u_color;
//    };
//    layout(location = 0) out vec4 o_color;
//
//    void main()
//    {
//        o_color = u_color;
//    }
//    )";
//    fragmentShaderStage.source = R"(
//        #include <metal_stdlib>
//        #include <simd/simd.h>
//
//        using namespace metal;
//
//        struct Color
//        {
//            float4 u_color;
//        };
//
//        struct main0_out
//        {
//            float4 o_color [[color(0)]];
//        };
//
//        fragment main0_out main0(constant Color& _12 [[buffer(0)]])
//        {
//            main0_out out = {};
//            out.o_color = _12.u_color;
//            return out;
//        }
//    )";
//#else
    
#if defined(USE_VULKAN) || defined(USE_METAL)
    fragmentShaderStage.source = R"(
    #ifdef GL_ES
                precision highp float;
    #endif
        layout(binding = 0) uniform Color
        {
            vec4 u_color;
        };
        layout(location = 0) out vec4 o_color;
    
        void main()
        {
            o_color = u_color;
        }
    )";
#elif defined(USE_GLES2)
    fragmentShaderStage.source = R"(
        precision highp float;
        uniform vec4 u_color;
        void main()
        {
            gl_FragColor = vec4(1, 1, 0, 1); // u_color;
        }
    )";
#else
    fragmentShaderStage.source = R"(
    #ifdef GL_ES
    precision highp float;
    #endif
    layout(std140) uniform Color
    {
        vec4 u_color;
    };
    out vec4 o_color;
    
    void main()
    {
        o_color = vec4(1, 1, 0, 1); // u_color;
    }
    )";
#endif // #ifdef USE_GLES2
    
//#endif // #if (CC_PLATFORM == CC_PLATFORM_MAC_OSX)
    shaderStageList.emplace_back(std::move(fragmentShaderStage));

    GFXUniformList uniformList = { { "u_color", GFXType::FLOAT4, 1 } };
    GFXUniformBlockList uniformBlockList = { { GFXShaderType::FRAGMENT, 0, "Color", uniformList } };

    GFXShaderInfo shaderInfo;
    shaderInfo.name = "Basic Triangle";
    shaderInfo.stages = std::move(shaderStageList);
    shaderInfo.blocks = std::move(uniformBlockList);
    _shader = _device->createShader(shaderInfo);
}

void BasicTriangle::createVertexBuffer()
{
    float ySign = _device->getProjectionSignY();

    float vertexData[] = {
        -0.5f, -0.5f * ySign,
        0.5f, -0.5f * ySign,
        0.0f, 0.5f * ySign,
        0.5f, 0.5f * ySign,
    };

    GFXBufferInfo vertexBufferInfo = {
          GFXBufferUsage::VERTEX,
          GFXMemoryUsage::DEVICE,
          2 * sizeof(float),
          sizeof(vertexData),
          GFXBufferFlagBit::NONE };

    _vertexBuffer = _device->createBuffer(vertexBufferInfo);
    _vertexBuffer->update(vertexData, 0, sizeof(vertexData));

    GFXBufferInfo uniformBufferInfo = {
           GFXBufferUsage::UNIFORM,
           GFXMemoryUsage::DEVICE | GFXMemoryUsage::HOST,
           4 * sizeof(float),
           sizeof(GFXColor),
           GFXBufferFlagBit::NONE };
     _uniformBuffer = _device->createBuffer(uniformBufferInfo);
    
    GFXDrawInfo drawInfo;
    drawInfo.firstIndex = 0;
    drawInfo.indexCount = 6;
    drawInfo.instanceCount = 1;
    
    GFXBufferInfo indirectBufferInfo = {
        GFXBufferUsageBit::INDIRECT,
        GFXMemoryUsage::DEVICE | GFXMemoryUsage::HOST,
        sizeof(GFXDrawInfo),
        sizeof(GFXDrawInfo),
        GFXBufferFlagBit::NONE
    };
    _indirectBuffer = _device->createBuffer(indirectBufferInfo);
    _indirectBuffer->update(&drawInfo, 0, sizeof(GFXDrawInfo));
    
    unsigned short indices[] = { 0,1,2,1,3,2};
    GFXBufferInfo indexBufferInfo  = {
        GFXBufferUsageBit::INDEX,
        GFXMemoryUsage::DEVICE,
        sizeof(unsigned short),
        sizeof(indices),
        GFXBufferFlagBit::NONE
    };
    _indexBuffer = _device->createBuffer(indexBufferInfo);
    _indexBuffer->update(indices, 0, sizeof(indices));

}

void BasicTriangle::createInputAssembler()
{
    GFXAttribute position = {"a_position", GFXFormat::RG32F, false, 0, false};
    GFXInputAssemblerInfo inputAssemblerInfo;
    inputAssemblerInfo.attributes.emplace_back(std::move(position));
    inputAssemblerInfo.vertexBuffers.emplace_back(_vertexBuffer);
    inputAssemblerInfo.indexBuffer = _indexBuffer;
    inputAssemblerInfo.indirectBuffer = _indirectBuffer;
    _inputAssembler = _device->createInputAssembler(inputAssemblerInfo);
}

void BasicTriangle::createPipeline()
{
    GFXBindingList bindingList = { { GFXShaderType::FRAGMENT, 0, GFXBindingType::UNIFORM_BUFFER, "u_color", 1 } };
    GFXBindingLayoutInfo bindingLayoutInfo = { bindingList };
    _bindingLayout = _device->createBindingLayout(bindingLayoutInfo);

    GFXPipelineLayoutInfo pipelineLayoutInfo;
    pipelineLayoutInfo.layouts = { _bindingLayout };
    _pipelineLayout = _device->createPipelineLayout(pipelineLayoutInfo);

    GFXPipelineStateInfo pipelineInfo;
    pipelineInfo.primitive = GFXPrimitiveMode::TRIANGLE_LIST;
    pipelineInfo.shader = _shader;
    pipelineInfo.inputState = { _inputAssembler->getAttributes() };
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _fbo->getRenderPass();

    _pipelineState = _device->createPipelineState(pipelineInfo);
}

void BasicTriangle::tick(float dt) {

    GFXRect render_area = {0, 0, _device->getWidth(), _device->getHeight() };
    _time += dt;
    GFXColor clear_color = {1.0f, 0, 0, 1.0f};
    
    GFXColor uniformColor;
    uniformColor.r = std::abs(std::sin(_time));
    uniformColor.g = 1.0f;
    uniformColor.b = 0.0f;
    uniformColor.a = 1.0f;

    _uniformBuffer->update(&uniformColor, 0, sizeof(uniformColor));
    _bindingLayout->bindBuffer(0, _uniformBuffer);
    _bindingLayout->update();

    _device->acquire();

    for (auto commandBuffer : _commandBuffers)
    {
        commandBuffer->begin();
        commandBuffer->beginRenderPass(_fbo, render_area, GFXClearFlagBit::ALL, std::move(std::vector<GFXColor>({clear_color})), 1.0f, 0);
        commandBuffer->bindInputAssembler(_inputAssembler);
        commandBuffer->bindPipelineState(_pipelineState);
        commandBuffer->bindBindingLayout(_bindingLayout);
        commandBuffer->draw(_inputAssembler);
        commandBuffer->endRenderPass();
        commandBuffer->end();
    }

    _device->getQueue()->submit(_commandBuffers);
    _device->present();
}

NS_CC_END
