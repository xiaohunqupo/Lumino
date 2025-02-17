﻿
#include "Internal.hpp"
#include <LuminoGraphics/GPU/Texture.hpp>
#include <LuminoGraphics/GPU/SamplerState.hpp>
#include <LuminoGraphics/Rendering/Material.hpp>
#include <LuminoGraphics/Rendering/CommandList.hpp>
#include <LuminoGraphics/Rendering/RenderingContext.hpp>
#include <LuminoGraphics/Rendering/RenderView.hpp>
#include <LuminoEngine/PostEffect/SSRPostEffect.hpp>
#include <LuminoGraphics/Rendering/detail/RenderingManager.hpp>

namespace ln {

//==============================================================================
// SSRPostEffect

SSRPostEffect::SSRPostEffect()
{
}

void SSRPostEffect::init()
{
    PostEffect::init();
}

Ref<PostEffectInstance> SSRPostEffect::onCreateInstance()
{
    return makeObject_deprecated<detail::SSRPostEffectInstance>(this);
}

//==============================================================================
// SSRPostEffectInstance

namespace detail {

SSRPostEffectCore::SSRPostEffectCore()
    : m_viewWidth(0)
    , m_viewHeight(0)
{
}

bool SSRPostEffectCore::init(Material* compositeMaterial)
{

    auto shader1 = RenderingManager::instance()->builtinShader(BuiltinShader::SSRRayTracing);
    m_ssrMaterial = makeObject_deprecated<Material>();
    m_ssrMaterial->setShader(shader1);
    m_ssrMaterial_ColorSampler = shader1->findParameter(_TT("_ColorSampler"));
    m_ssrMaterial_NormalAndDepthSampler = shader1->findParameter(_TT("_NormalAndDepthSampler"));
    m_ssrMaterial_ViewDepthSampler = shader1->findParameter(_TT("_ViewDepthSampler"));
    m_ssrMaterial_MetalRoughSampler = shader1->findParameter(_TT("_MetalRoughSampler"));

    auto shader2 = RenderingManager::instance()->builtinShader(BuiltinShader::SSRBlur);
    m_ssrBlurMaterial1 = makeObject_deprecated<Material>();
    m_ssrBlurMaterial1->setShader(shader2);

    m_ssrBlurMaterial2 = makeObject_deprecated<Material>();
    m_ssrBlurMaterial2->setShader(shader2);

    if (!compositeMaterial) {
        auto shader3 = RenderingManager::instance()->builtinShader(BuiltinShader::SSRComposite);
        m_ssrCompositeMaterial = makeObject_deprecated<Material>();
        m_ssrCompositeMaterial->setShader(shader3);
        m_paramColorSampler = shader3->findParameter(_TT("_ColorSampler"));
        m_paramSSRSampler = shader3->findParameter(_TT("_SSRSampler"));
    }

    // TODO: 他と共有したいところ
    m_samplerState = makeObject_deprecated<SamplerState>(TextureFilterMode::Linear, TextureAddressMode::Clamp);

    return true;
}

bool SSRPostEffectCore::prepare(RenderView* renderView, CommandList* context, RenderTargetTexture* source)
{
    Texture* viewNormalMap = renderView->builtinRenderTexture(BuiltinRenderTextureType::ViewNormalMap);
    Texture* viewDepthMap = renderView->builtinRenderTexture(BuiltinRenderTextureType::ViewDepthMap);
    Texture* viewMaterialMap = renderView->builtinRenderTexture(BuiltinRenderTextureType::ViewMaterialMap);

    if (viewNormalMap && viewDepthMap && viewMaterialMap) {

        int resx = source->width();
        int resy = source->height();
        if (m_viewWidth != resx || m_viewHeight != resy) {
            resetResources(resx, resy);
        }

#if LN_USE_KANATA
        // TODO: 毎回名前検索ではなく最適化したい
        m_ssrMaterial->setTexture(_TT("_ColorSampler"), source);
        m_ssrMaterial->setTexture(_TT("_NormalAndDepthSampler"), viewNormalMap);
        m_ssrMaterial->setTexture(_TT("_ViewDepthSampler"), viewDepthMap);
        m_ssrMaterial->setTexture(_TT("_MetalRoughSampler"), viewMaterialMap);
#else
        m_ssrMaterial_ColorSampler->setTexture(source);
        m_ssrMaterial_NormalAndDepthSampler->setTexture(viewNormalMap);
        m_ssrMaterial_ViewDepthSampler->setTexture(viewDepthMap);
        m_ssrMaterial_MetalRoughSampler->setTexture(viewMaterialMap);
#endif
        context->blit(m_ssrMaterial, m_ssrTarget);

#if 0   // DEBUG: Ray-trace result
        context->blit(m_ssrMaterial, destination);
        return true;
#endif
        m_ssrBlurMaterial1->setMainTexture(m_ssrTarget);
        context->blit(m_ssrBlurMaterial1, m_blurTarget1);

        m_ssrBlurMaterial2->setMainTexture(m_blurTarget1);
        context->blit(m_ssrBlurMaterial2, m_blurTarget2);

        return true;
    }
    else {
        return false;
    }
}

void SSRPostEffectCore::render(CommandList* context, RenderTargetTexture* source, RenderTargetTexture* destination)
{
#if LN_USE_KANATA
    // TODO: 毎回名前検索ではなく最適化したい
    m_ssrCompositeMaterial->setTexture(_TT("_ColorSampler"), source);
    m_ssrCompositeMaterial->setTexture(_TT("_SSRSampler"), m_blurTarget2);
#else
    m_paramColorSampler->setTexture(source);
    m_paramSSRSampler->setTexture(m_blurTarget2);
#endif
    context->blit(m_ssrCompositeMaterial, destination);
}

void SSRPostEffectCore::resetResources(int resx, int resy)
{
    // TODO: tempolary からとっていいかも
    m_ssrTarget = makeObject_deprecated<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false, false);
    m_ssrTarget->setSamplerState(m_samplerState);
    m_blurTarget1 = makeObject_deprecated<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false, false);
    m_blurTarget1->setSamplerState(m_samplerState);
    m_blurTarget2 = makeObject_deprecated<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false, false);
    m_blurTarget2->setSamplerState(m_samplerState);
    m_compositeTarget = makeObject_deprecated<RenderTargetTexture>(resx, resy, TextureFormat::RGBA8, false, false);
    m_compositeTarget->setSamplerState(m_samplerState);

    m_viewWidth = resx;
    m_viewHeight = resy;
}

} // namespace detail

//==============================================================================
// SSRPostEffectInstance

namespace detail {

SSRPostEffectInstance::SSRPostEffectInstance()
    : m_owner(nullptr)
{
}

bool SSRPostEffectInstance::init(SSRPostEffect* owner)
{
    if (!PostEffectInstance::init()) return false;

    m_owner = owner;

    if (!m_effect.init(nullptr)) {
        return false;
    }

    return true;
}

bool SSRPostEffectInstance::onRender(RenderView* renderView, CommandList* context, RenderTargetTexture* source, RenderTargetTexture* destination)
{
    if (m_effect.prepare(renderView, context, source)) {
        m_effect.render(context, source, destination);
        return true;
    }
    else {
        return false;
    }
}

} // namespace detail
} // namespace ln

