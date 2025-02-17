﻿#pragma once
#include "GLHelper.hpp"

namespace ln {
namespace detail {
class OpenGLDevice;
class GLShaderDescriptorTable;
class GLUniformBuffer;
class GLSamplerState;

class GLSLShader {
public:
    GLSLShader();
    ~GLSLShader();
    Result<> create(const byte_t* code, int length, GLenum type, ShaderCompilationDiag* diag);
    void dispose();

    GLuint shader() const { return m_shader; }
    GLenum type() const { return m_type; }

private:
    GLuint m_shader;
    GLenum m_type;
};

class GLShaderPass
    : public IShaderPass {
public:
    GLShaderPass();
    virtual ~GLShaderPass();
    Result<> init(OpenGLDevice* context, const ShaderPassCreateInfo& createInfo, const byte_t* vsCode, int vsCodeLen, const byte_t* fsCodeLen, int psCodeLen, ShaderCompilationDiag* diag);
    virtual void onDestroy() override;

    GLuint program() const { return m_program; }
    const GLShaderDescriptorTable* layout() const { return m_descriptorTable.get(); }
    void apply() const;

private:
    OpenGLDevice* m_context;
    GLuint m_program;
    Ref<GLShaderDescriptorTable> m_descriptorTable;
};

// GLDescriptor を Bind するためのクラス。
// GLShaderPass と 1:1 で使う。
class GLShaderDescriptorTable : public RefObject {
public:
    struct UniformBufferInfo {
        GLuint bindingPoint = 0; // こちらは自由に決めていい。
        GLuint blockIndex = 0;   // これが layout(binding=) と一致させる値
        GLint blockSize;

        // null になることもある。DescriptorLayout 側のインデックスと一致させるため、
        // UniformBufferInfo のインスタンスは作られるが、Active な UBO ではない場合 0 になる。
        //GLuint ubo = 0;
        //GLUniformBuffer* buffer;
        //size_t offset;
        //size_t size;

        // DescriptorLayout の UniformBuffer index.
        // Descriptor からデータを取り出すときに使う。
        kokage::LayoutSlotIndex layoutSlotIndex; 

        // glCompileShader() でランタイムコンパイルした Shader の各 ActiveUniformBlock のうち、実際には使われていないものに Bind するダミーの UBO。
        // ActiveUniformBlock には、定義だけあるが実際には参照していないものも含む。kokage はそういった Buffer を見つけても、Layout 情報として出力しない。
        // しかし WebGL では、そのような参照されていない UBO に対しても、十分なサイズを持つ UBO をバインドしないと GL_INVALID_OPERATION が発生する。
        // https://github.com/mikolalysenko/angle/blob/master/src/libANGLE/validationES.cpp#L1772-L1799
        // 今のところ WebGL でのみ確認しており、DX12 や Vulkan では Bind しなくてもエラーにはならない。
        // 無駄に VRAM を消費するのもイヤなので、OpenGL バックエンド側でダミーを持たせることで対策している。
        GLuint deactiveUBO = 0;
    };

    // textureXD と samplerState を結合するためのデータ構造
    struct ResouceUniformInfo {
        //std::string name; // lnCISlnTOg_texture1lnSOg_samplerState1 のような FullName
        GLint uniformLocation = -1;
        kokage::LayoutSlotIndex layoutSlotIndex; // Index of DescriptorLayout::textureRegister.
        //GLint isRenderTargetUniformLocation = -1; // texture または sampler の場合、それが RenderTarget であるかを示す値を入力する Uniform の Loc。末尾が lnIsRT になっているもの。
        //int m_textureExternalUnifromIndex = -1;   // Index of m_externalTextureUniforms
        //int m_samplerExternalUnifromIndex = -1;   // Index of m_externalSamplerUniforms
    };

    GLShaderDescriptorTable();
    bool init(const GLShaderPass* ownerPass, const kokage::DescriptorLayout* descriptorLayout);
    void dispose();
    void setData(const ShaderDescriptorTableUpdateInfo* data);
    void bind(GLuint program);

    const std::vector<UniformBufferInfo>& bufferInfos() const { return m_uniformBuffers; }
    const std::vector<ResouceUniformInfo>& resourceInfos() const { return m_resourceUniforms; }

private:
    void addResourceUniform(const std::string& name, GLint uniformLocation, const kokage::DescriptorLayout* descriptorLayout);
    //void addIsRenderTargetUniform(const std::string& name, GLint uniformLocation);

    // 以下、要素番号は DescriptorLayout の各メンバの要素番号と一致する。bindingIndex ではない。
    std::vector<UniformBufferInfo> m_uniformBuffers;
    std::vector<ResouceUniformInfo> m_resourceUniforms;
    //std::vector<ExternalUnifrom> m_externalTextureUniforms;
    //std::vector<ExternalUnifrom> m_externalSamplerUniforms;
};

#if 0
class GLShaderDescriptorTable
    : public IShaderDescriptorTable {
public:
    struct UniformBufferInfo {
        GLuint bindingPoint = 0; // こちらは自由に決めていい。
        GLuint blockIndex = 0;   // これが layout(binding=) と一致させる値
        GLint blockSize;

        // null になることもある。DescriptorLayout 側のインデックスと一致させるため、
        // UniformBufferInfo のインスタンスは作られるが、Active な UBO ではない場合 0 になる。
        //GLuint ubo = 0;
        GLUniformBuffer* buffer;
        size_t offset;
        //size_t size;
    };

    // textureXD と samplerState を結合するためのデータ構造
    struct SamplerUniformInfo {
        std::string name; // lnCISlnTOg_texture1lnSOg_samplerState1 のような FullName
        GLint uniformLocation = -1;
        GLint isRenderTargetUniformLocation = -1; // texture または sampler の場合、それが RenderTarget であるかを示す値を入力する Uniform の Loc。末尾が lnIsRT になっているもの。
        int m_textureExternalUnifromIndex = -1;   // Index of m_externalTextureUniforms
        int m_samplerExternalUnifromIndex = -1;   // Index of m_externalSamplerUniforms
    };

    // 外部に公開する Uniform 情報。
    // lnCISlnTOg_texture1lnSOg_samplerState1 は、g_texture1 と g_samplerState1 の２つの uniform であるかのように公開する。
    struct ExternalUnifrom {
        std::string name; // g_texture1, g_samplerState1 といった、ユーザーが定義した uniform 名
        GLTextureBase* texture = nullptr;
        GLSamplerState* samplerState = nullptr;
    };

    GLShaderDescriptorTable();
    bool init(const GLShaderPass* ownerPass, const DescriptorLayout* descriptorLayout);
    void dispose() override;
    void setData(const ShaderDescriptorTableUpdateInfo* data);
    void bind(GLuint program);

private:
    void addGlslSamplerUniform(const std::string& name, GLint uniformLocation, const DescriptorLayout* descriptorLayout);
    void addIsRenderTargetUniform(const std::string& name, GLint uniformLocation);

    // 以下、要素番号は DescriptorLayout の各メンバの要素番号と一致する。bindingIndex ではない。
    std::vector<UniformBufferInfo> m_uniformBuffers;
    std::vector<SamplerUniformInfo> m_samplerUniforms;
    std::vector<ExternalUnifrom> m_externalTextureUniforms;
    std::vector<ExternalUnifrom> m_externalSamplerUniforms;
};
#endif

} // namespace detail
} // namespace ln
