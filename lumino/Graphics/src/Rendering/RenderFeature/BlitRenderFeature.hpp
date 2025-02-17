﻿#pragma once
#include <LuminoGraphics/Rendering/RenderFeature.hpp>
#include "../RLIs/RLIBatchList.hpp"

namespace ln {
class VertexLayout;
class VertexBuffer;
namespace detail {
class RenderingManager;

// MeshSection 単位で描画する
class BlitRenderFeature
	: public RenderFeature
{
public:
    BlitRenderFeature();
	void init(RenderingManager* manager);

	RequestBatchResult blit(RenderFeatureBatchList* batchList, const RLIBatchState& batchState);

	virtual void submitBatch(GraphicsCommandList* context, detail::RenderFeatureBatchList* batchList) override;
	virtual void renderBatch(GraphicsCommandList* context, RenderFeatureBatch* batch) override;

private:
	//class Batch : public RenderFeatureBatch
	//{
	//public:
	//	int requestedCount;
	//};

	RenderingManager* m_manager;
    Ref<VertexLayout> m_vertexLayout;
	Ref<VertexBuffer> m_vertexBuffer;
	int m_requestedCount;
};


} // namespace detail
} // namespace ln

