﻿#include "Internal.hpp"
#include <LuminoGraphics/GPU/GraphicsCommandBuffer.hpp>
#include <LuminoGraphics/GPU/VertexLayout.hpp>
#include <LuminoGraphics/GPU/VertexBuffer.hpp>
#include <LuminoGraphics/Font/Font.hpp>
#include <LuminoGraphics/Rendering/Material.hpp>
#include <LuminoGraphics/Rendering/CanvasContext.hpp>
#include <LuminoGraphics/Rendering/RenderingContext.hpp>
#include <LuminoGraphics/Rendering/RenderView.hpp>
#include <LuminoGraphics/Rendering/CommandList.hpp>
#include <LuminoGraphics/Mesh/MeshPrimitive.hpp>
#include <LuminoGraphics/Mesh/SkinnedMeshModel.hpp>
#include <LuminoGraphics/Mesh/MeshModeEntity.hpp>
#include <LuminoGraphics/Font/detail/FontManager.hpp>
#include "../Mesh/MeshGeneraters/MeshGenerater.hpp"
#include "CommandListServer.hpp"
#include <LuminoGraphics/Rendering/detail/RenderingManager.hpp>
#include "DrawElementListBuilder.hpp"

namespace ln {

//==============================================================================
// RenderingContext
/*
RenderingContext と RenderView は違うもの？
----------
RenderView は視点の数(=カメラの数) だけ存在する。
RenderingContext は RenderView の描画のたびにリセットがかかるので、1つのインスタンスを使いまわしてOK。

[2022/4/22] リセットのタイミングについて
----------
RenderView でリセットする。
ただし、今のところ World の中に UI を描画するなど、 RenderView のネストを計画している。
その場合、子 RenderView は "Sub-RenderView" のようなフラグを付けておき、これの有無でリセット掛けるかを判断してみたい。

*/

RenderingContext::RenderingContext()
    : m_manager(detail::RenderingManager::instance())
    , m_pathContext(makeObject_deprecated<CanvasContext>())
    , m_pathBegan(false)
    , m_commandList(nullptr)
    , m_listServer(makeRef<detail::CommandListServer>()) {
    m_commandList = m_listServer->acquirePrimaryList(RenderPart::Geometry, nullptr);
}

Result<> RenderingContext::init() {
    if (!Object::init()) return err();
    return ok();
}

void RenderingContext::resetForBeginRendering(const RenderViewPoint* viewPoint) {
    m_listServer->clearCommandsAndState(viewPoint);
    m_dynamicLightInfoList.clear();
}

void RenderingContext::setRenderPass(RenderPass* value) {
    m_commandList->setRenderPass(value);
}

RenderPass* RenderingContext::renderPass() const {
    return m_commandList->renderPass();
}

void RenderingContext::setViewportRect(const RectI& value) {
    m_commandList->setViewportRect(value);
}

void RenderingContext::setScissorRect(const RectI& value) {
    m_commandList->setScissorRect(value);
}

void RenderingContext::setTransfrom(const Matrix& value) {
    m_commandList->setTransfrom(value);
}

void RenderingContext::setBlendMode(Optional_deprecated<BlendMode> value) {
    m_commandList->setBlendMode(value);
}

void RenderingContext::setShadingModel(Optional_deprecated<ShadingModel> value) {
    m_commandList->setShadingModel(value);
}

void RenderingContext::setCullingMode(Optional_deprecated<CullMode> value) {
    m_commandList->setCullingMode(value);
}

void RenderingContext::setDepthTestEnabled(Optional_deprecated<bool> value) {
    m_commandList->setDepthTestEnabled(value);
}

void RenderingContext::setDepthWriteEnabled(Optional_deprecated<bool> value) {
    m_commandList->setDepthWriteEnabled(value);
}

void RenderingContext::setOpacity(float value) {
    m_commandList->setOpacity(value);
}

void RenderingContext::setMaterial(Material* material) {
    m_commandList->setMaterial(material);
}

void RenderingContext::setRenderPhase(RenderPart value) {
    m_commandList->setRenderPhase(value);
}

void RenderingContext::setColorScale(const Color& value) {
    m_commandList->setColorScale(value);
}

void RenderingContext::setBlendColor(const Color& value) {
    m_commandList->setBlendColor(value);
}

void RenderingContext::setTone(const ColorTone& value) {
    m_commandList->setTone(value);
}

void RenderingContext::setFont(Font* value) {
    m_commandList->setFont(value);
}

void RenderingContext::setTextColor(const Color& value) {
    m_commandList->setTextColor(value);
}

void RenderingContext::resetState() {
    m_commandList->resetState();
}

void RenderingContext::pushState(bool reset) {
    m_commandList->pushState(reset);
}

void RenderingContext::popState() {
    m_commandList->popState();
}

void RenderingContext::drawBatchProxy(kanata::BatchProxy* batchProxy) {
    m_commandList->drawBatchProxy(batchProxy);
}

void RenderingContext::clear(Flags<ClearFlags> flags, const Color& color, float z, uint8_t stencil) {
    m_commandList->clear(flags, color, z, stencil);
}

void RenderingContext::drawLine(const Vector3& from, const Color& fromColor, const Vector3& to, const Color& toColor) {
    m_commandList->drawLine(from, fromColor, to, toColor);
}

//void RenderingContext::drawPlane(Material* material, float width, float depth, const Color& color) {
//    m_commandList->drawPlane(material, width, depth, Vector2::Zero, Vector2::Ones, color);
//}
//
//void RenderingContext::drawPlane(Material* material, float width, float depth, const Vector2& uv1, const Vector2& uv2, const Color& color) {
//    m_commandList->drawPlane(material, width, depth, uv1, uv2, color);
//}
//
//void RenderingContext::drawSphere(Material* material, float radius, int slices, int stacks, const Color& color, const Matrix& localTransform) {
//    m_commandList->drawSphere(material, radius, slices, stacks, color, localTransform);
//}
//
//void RenderingContext::drawBox(const Box& box, const Color& color, const Matrix& localTransform) {
//    m_commandList->drawBox(box, color, localTransform);
//}

void RenderingContext::drawRegularPolygonPrimitive(int vertexCount, float radius, const Color& color, bool fill, const Matrix& localTransform) {
    m_commandList->drawRegularPolygonPrimitive(vertexCount, radius, color, fill, localTransform);
}

//void RenderingContext::drawScreenRectangle() {
//    m_commandList->drawScreenRectangle();
//}

void RenderingContext::blit(Material* source, RenderTargetTexture* destination/*, RenderPart phase*/) {
    m_commandList->blit(source, destination/*, phase*/);
}

//void RenderingContext::drawSprite(
//    const Matrix& transform,
//    const Size& size,
//    const Vector2& anchor,
//    const Rect& srcRect,
//    const Color& color,
//    SpriteBaseDirection baseDirection,
//    BillboardType billboardType,
//    const Flags<SpriteFlipFlags>& flipFlags,
//    Material* material) {
//    m_commandList->drawSprite(
//        transform,
//        size,
//        anchor,
//        srcRect,
//        color,
//        baseDirection,
//        billboardType,
//        flipFlags,
//        material);
//}

void RenderingContext::drawPrimitive(VertexLayout* vertexDeclaration, VertexBuffer* vertexBuffer, PrimitiveTopology topology, int startVertex, int primitiveCount) {
    m_commandList->drawPrimitive(vertexDeclaration, vertexBuffer, topology, startVertex, primitiveCount);
}

// LOD なし。というか直接描画
void RenderingContext::drawMesh(MeshResource* meshResource, int sectionIndex) {
    m_commandList->drawMesh(meshResource, sectionIndex);
}

void RenderingContext::drawMesh(MeshPrimitive* mesh, int sectionIndex) {
    m_commandList->drawMesh(mesh, sectionIndex);
}

void RenderingContext::drawSkinnedMesh(MeshPrimitive* mesh, int sectionIndex, detail::SkeletonInstance* skeleton, detail::MorphInstance* morph) {
    m_commandList->drawSkinnedMesh(mesh, sectionIndex, skeleton, morph);
}

void RenderingContext::drawMeshInstanced(Material* material, InstancedMeshList* list) {
    m_commandList->drawMeshInstanced(material, list);
}

void RenderingContext::drawTextSprite(const StringView& text, const Color& color, const Vector2& anchor, SpriteBaseDirection baseDirection, detail::FontRequester* font) {
    m_commandList->drawTextSprite(text, color, anchor, baseDirection, font);
}

void RenderingContext::drawText(const StringView& text, const Rect& area, TextAlignment alignment /*, TextCrossAlignment crossAlignment*/ /*, const Color& color, Font* font*/) {
    m_commandList->drawText(text, area, alignment);
}

void RenderingContext::drawChar(uint32_t codePoint, const Color& color, Font* font, const Matrix& transform) {
    m_commandList->drawChar(codePoint, color, font, transform);
}

void RenderingContext::drawFlexGlyphRun(detail::FlexGlyphRun* glyphRun) {
    m_commandList->drawFlexGlyphRun(glyphRun);
}

void RenderingContext::invokeExtensionRendering(INativeGraphicsExtension* extension) {
    m_commandList->invokeExtensionRendering(extension);
}

CanvasContext* RenderingContext::beginPath() {
    if (LN_REQUIRE(!m_pathBegan)) return nullptr;
    m_pathBegan = true;
    return m_pathContext;
}

void RenderingContext::endPath() {
    if (LN_REQUIRE(m_pathBegan)) return;
    m_pathBegan = false;
    m_commandList->drawPath(m_pathContext);
}

void RenderingContext::addAmbientLight(const Color& color, float intensity) {
    addDynamicLightInfo(detail::DynamicLightInfo::makeAmbientLightInfo(color, intensity));
}

void RenderingContext::addHemisphereLight(const Color& skyColor, const Color& groundColor, float intensity) {
    addDynamicLightInfo(detail::DynamicLightInfo::makeHemisphereLightInfo(skyColor, groundColor, intensity));
}

void RenderingContext::addEnvironmentLightInfo(const Color& color, const Color& ambientColor, const Color& skyColor, const Color& groundColor, float intensity, const Vector3& direction, bool mainLight, float shadowCameraZFar, float shadowLightZFar) {
    addDynamicLightInfo(detail::DynamicLightInfo::makeEnvironmentLightInfo(color, ambientColor, skyColor, groundColor, intensity, direction, mainLight, shadowCameraZFar, shadowLightZFar));
}

void RenderingContext::addDirectionalLight(const Color& color, float intensity, const Vector3& direction, bool mainLight, float shadowCameraZFar, float shadowLightZFar) {
    addDynamicLightInfo(detail::DynamicLightInfo::makeDirectionalLightInfo(color, intensity, direction, mainLight, shadowCameraZFar, shadowLightZFar));
}

void RenderingContext::addPointLight(const Color& color, float intensity, const Vector3& position, float range, float attenuation) {
    addDynamicLightInfo(detail::DynamicLightInfo::makePointLightInfo(color, intensity, position, range, attenuation));
}

void RenderingContext::addSpotLight(const Color& color, float intensity, const Vector3& position, const Vector3& direction, float range, float attenuation, float spotAngle, float spotPenumbra) {
    addDynamicLightInfo(detail::DynamicLightInfo::makeSpotLightInfo(color, intensity, position, direction, range, attenuation, spotAngle, spotPenumbra));
}

Size RenderingContext::measureTextSize(Font* font, const StringView& text) const {
    if (LN_REQUIRE(font)) return Size::Zero;
    if (text.isEmpty()) return Size::Zero;
    return font->measureRenderSize(text, viewPoint()->dpiScale);
}

Size RenderingContext::measureTextSize(Font* font, uint32_t codePoint) const {
    if (LN_REQUIRE(font)) return Size::Zero;
    if (codePoint == 0) return Size::Zero;
    return font->measureRenderSize(codePoint, viewPoint()->dpiScale);
}

CommandList* RenderingContext::getCommandList(RenderPart index1) {
    return m_listServer->acquirePrimaryList(index1, viewPoint());
}

const RenderViewPoint* RenderingContext::viewPoint() const {
    return m_commandList->viewPoint();
}

void RenderingContext::setBaseTransfrom(const Optional_deprecated<Matrix>& value) {
    m_commandList->setBaseTransfrom(value);
}

const Matrix& RenderingContext::baseTransform() const {
    return m_commandList->baseTransform();
}

void RenderingContext::setRenderPriority(int value) {
    m_commandList->setRenderPriority(value);
}

void RenderingContext::setBaseBuiltinEffectData(const Optional_deprecated<detail::BuiltinEffectData>& value) {
    m_commandList->setBaseBuiltinEffectData(value);
}

void RenderingContext::setAdditionalElementFlags(detail::RenderDrawElementTypeFlags value) {
    m_commandList->setAdditionalElementFlags(value);
}

void RenderingContext::setObjectId(int value) {
    m_commandList->setObjectId(value);
}

const Ref<detail::DrawElementListBuilder>& RenderingContext::builder() const {
    return m_commandList->builder();
}

} // namespace ln
