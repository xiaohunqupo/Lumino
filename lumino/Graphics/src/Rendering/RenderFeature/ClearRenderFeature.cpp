﻿
#include "Internal.hpp"
#include <LuminoGraphics/detail/GraphicsManager.hpp>
#include <LuminoGraphics/GPU/VertexLayout.hpp>
#include <LuminoGraphics/GPU/VertexBuffer.hpp>
#include <LuminoGraphics/GPU/GraphicsCommandBuffer.hpp>
#include <LuminoGraphics/Rendering/Vertex.hpp>
#include <LuminoGraphics/Rendering/detail/RenderingManager.hpp>
#include "ClearRenderFeature.hpp"

namespace ln {
namespace detail {

//==============================================================================
// ClearRenderFeature

RequestBatchResult ClearRenderFeature::clear(RenderFeatureBatchList* batchList, const RLIBatchState& batchState, ClearFlags flags, const Color& color, float depth, uint8_t stencil)
{
	auto* batch = batchList->addNewBatch<ClearBatch>(this, batchState);
	batch->data.flags = flags;
	batch->data.color = color;
	batch->data.depth = depth;
	batch->data.stencil = stencil;
	return RequestBatchResult::Submitted;
}

void ClearRenderFeature::submitBatch(GraphicsCommandList* context, detail::RenderFeatureBatchList* batchList) {
	LN_UNREACHABLE();
}

void ClearRenderFeature::renderBatch(GraphicsCommandList* context, detail::RenderFeatureBatch* batch) {
	auto localBatch = static_cast<ClearBatch*>(batch);
	context->clear(localBatch->data.flags, localBatch->data.color, localBatch->data.depth, localBatch->data.stencil);
}

} // namespace detail
} // namespace ln

