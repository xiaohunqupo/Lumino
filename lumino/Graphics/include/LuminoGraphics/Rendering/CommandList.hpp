﻿#pragma once
#include <LuminoEngine/Graphics/RenderState.hpp>
#include <LuminoEngine/Graphics/ColorStructs.hpp>
#include <LuminoGraphics/Font/Common.hpp>
#include "Common.hpp"
#include "Kanata/Common.hpp"

namespace ln {

class CommandList
    : public Object {
public:
    //--------------------------------------------------------------------------
    /** @name render targets */
    /** @{ */

    void setRenderPass(RenderPass* value);
    RenderPass* renderPass() const;

    ///** @see RenderingContext::setRenderTarget */
    //void setRenderTarget(int index, RenderTargetTexture* value);

    ///** @see RenderingContext::renderTarget */
    //RenderTargetTexture* renderTarget(int index) const;

    ///** @see RenderingContext::setDepthBuffer */
    //void setDepthBuffer(DepthBuffer* value);

    /** @see RenderingContext::setViewportRect */
    void setViewportRect(const RectI& value);

    /** @see RenderingContext::setScissorRect */
    void setScissorRect(const RectI& value);

    /** @} */

    void setTransfrom(const Matrix& value);

    //--------------------------------------------------------------------------
    /** @name render status */
    /** @{ */

    // このあたりの設定は Material の設定をオーバーライドする。
    // 主な用途としては、Mesh のように複数のマテリアルを含むオブジェクトを描画する際、
    // サブセット全体の設定をオーバーライドするようなケースを想定している。
    // (特に、ヒットエフェクトのための BlendColor や、エディタでの両面表示設定など、Material 自体のパラメータを変更したくない場合)

    /** BlendMode を設定します。 シーンの既定値を使用する場合は nullptr を指定します。 */
    void setBlendMode(Optional_deprecated<BlendMode> value);

    /** ShadingModel を設定します。 シーンの既定値を使用する場合は nullptr を指定します。 */
    void setShadingModel(Optional_deprecated<ShadingModel> value);

    /** CullMode を設定します。 シーンの既定値を使用する場合は nullptr を指定します。 */
    void setCullingMode(Optional_deprecated<CullMode> value);

    /** 深度テストの有無を設定します。 シーンの既定値を使用する場合は nullptr を指定します。 */
    void setDepthTestEnabled(Optional_deprecated<bool> value);

    /** 深度書き込みの有無を設定します。 シーンの既定値を使用する場合は nullptr を指定します。 */
    void setDepthWriteEnabled(Optional_deprecated<bool> value);

    /** @} */

    // デフォルトは nullptr。この場合、既定のマテリアルが使用される。
    void setMaterial(Material* material);

    void setRenderPhase(RenderPart value);

    // BuiltinEffectData
    //void setTransfrom(const Matrix& value);
    void setOpacity(float value);
    void setColorScale(const Color& value);
    void setBlendColor(const Color& value);
    void setTone(const ColorTone& value);

    // Text
    void setFont(Font* value);
    void setTextColor(const Color& value);

    void resetState();
    void pushState(bool reset = true); // TODO: enum, scoped_guard
    void popState();

    //--------------------------------------------------------------------------
    /** @name drawing */
    /** @{ */

    void drawBatchProxy(kanata::BatchProxy* batchProxy);

    // TODO: deprecated. シャドウパスなど、マルチパスの SceneRenderer を実行するとき、メイン以外のパスでもクリアしてしまうので、対策するか、いっそ無くす。
    void clear(Flags<ClearFlags> flags = ClearFlags::All, const Color& color = Color::Zero, float z = 1.0f, uint8_t stencil = 0x00);

    void drawLine(const Vector3& from, const Color& fromColor, const Vector3& to, const Color& toColor);
    void drawLineList(const Vector3* points, int pointCount, const Color& color);
    void drawLineStripPrimitive(int pointCount, const Vector3* points, const Color* colors);
    //void drawPlane(Material* material, float width, float depth, const Color& color = Color::White);
    //void drawPlane(Material* material, float width, float depth, const Vector2& uv1, const Vector2& uv2, const Color& color = Color::White);
    //void drawSphere(Material* material, float radius, int slices, int stacks, const Color& color, const Matrix& localTransform = Matrix());
    //void drawBox(const Box& box, const Color& color = Color::White, const Matrix& localTransform = Matrix());

    /** 四辺の位置が -1.0~1.0 の矩形を描画します。座標変換行列に単位行列を使用することで、スクリーン全体を覆う矩形を描画することができます。 */
    //void drawScreenRectangle();

    // これは主に Post effect の実装で使用します。
    // 実際に処理が行われるのはレンダリングパイプラインの PostEffect フェーズです。
    // 通常、drawMesh や drawSprite とは実行されるタイミングが異なるため、Post effect の実装のみを目的として使用してください。
    //void blit(Material* material);
    //void blit(RenderTargetTexture* source, RenderTargetTexture* destination);
    //void blit(RenderTargetTexture* source, RenderTargetTexture* destination, Material* material);
    void blit(Material* source, RenderTargetTexture* destination/*, RenderPart phase = RenderPart::PostEffect*/);

    /** スプライトを描画します。 */
    //void drawSprite(
    //    const Matrix& transform,
    //    const Size& size,
    //    const Vector2& anchor,
    //    const Rect& srcRect,
    //    const Color& color,
    //    SpriteBaseDirection baseDirection,
    //    BillboardType billboardType,
    //    const Flags<SpriteFlipFlags>& flipFlags,
    //    Material* material);

    // TODO: GraphicsのRenderPass内の map 禁止に伴い。Rendering で直接 VertexBuffer や IndexBuffer を扱うことを禁止したほうがいいかも。
    // 代わりに、Mesh を使う。
    // VertexBuffer::map の意味が直接転送に変わるので、(mapImmediatly() とかにしたい) まずこれを使うわけにはいかない。
    // そうすると GraphicsCommandList::map を使ってコマンドバッファに乗せて転送することになるけど、GraphicsContext は RenderingContext の内側に隠蔽されているので
    // 直接触ることはできない。（やってしまうと、コマンド順序が間違う）
    void drawPrimitive(VertexLayout* vertexDeclaration, VertexBuffer* vertexBuffer, PrimitiveTopology topology, int startVertex, int primitiveCount);

    void drawMesh(MeshResource* meshResource, int sectionIndex);
    //void drawMesh(MeshContainer* meshContainer, int sectionIndex);
    void drawMesh(MeshPrimitive* mesh, int sectionIndex);
    void drawSkinnedMesh(MeshPrimitive* mesh, int sectionIndex, detail::SkeletonInstance* skeleton, detail::MorphInstance* morph);

    void drawMeshInstanced(Material* material, InstancedMeshList* list);

    void drawTextSprite(const StringView& text, const Color& color, const Vector2& anchor, SpriteBaseDirection baseDirection, detail::FontRequester* font);

    // font が nullptr の場合は defaultFont
    void drawText(const StringView& text, const Rect& area = Rect(), TextAlignment alignment = TextAlignment::Forward /*, TextCrossAlignment crossAlignment = TextCrossAlignment::Forward*/ /*, const Color& color, Font* font = nullptr*/);
    void drawChar(uint32_t codePoint, const Color& color, Font* font = nullptr, const Matrix& transform = Matrix::Identity);

    void drawFlexGlyphRun(detail::FlexGlyphRun* glyphRun);

    void drawPath(CanvasContext* context);

    //CanvasContext* beginPath();
    //void endPath();

    void invokeExtensionRendering(INativeGraphicsExtension* extension);

    // デバッグ2D用 AA無しプリミティブ

    // 線分または塗りつぶしで描かれる正多角形。
    // XY 平面上に、Z-正面で作成する。
    void drawRegularPolygonPrimitive(int vertexCount, float radius, const Color& color, bool fill, const Matrix& localTransform = Matrix());

    /** @} */

    // TODO: internal
    void clearCommandsAndState(const RenderViewPoint* viewPoint);
    //void resolveSingleFrameBatchProxies();
    const Ref<detail::DrawElementListBuilder>& builder() const { return m_builder; }
    const Ref<detail::DrawElementList>& elementList() const { return m_elementList; }
    const URef<kanata::BatchProxyCollector>& batchProxyCollector() const { return m_batchProxyCollector; }
    const URef<kanata::BatchCollector>& batchCollector() const { return m_batchCollector; }

    const RenderViewPoint* viewPoint() const;
    void setBaseTransfrom(const Optional_deprecated<Matrix>& value);
    const Matrix& baseTransform() const;
    void setRenderPriority(int value);
    void setBaseBuiltinEffectData(const Optional_deprecated<detail::BuiltinEffectData>& value);
    void setAdditionalElementFlags(detail::RenderDrawElementTypeFlags value);
    void setObjectId(int value);

LN_CONSTRUCT_ACCESS:
    CommandList();
    ~CommandList();

private:
    //void applyBatchMaterial(kanata::Batch* batch);

    detail::RenderingManager* m_manager;
    Ref<detail::DrawElementList> m_elementList; // TODO: もういらない
    Ref<detail::DrawElementListBuilder> m_builder;
    URef<kanata::BatchProxyCollector> m_batchProxyCollector;
    URef<kanata::BatchCollector> m_batchCollector;
};

} // namespace ln
