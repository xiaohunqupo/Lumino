﻿
#include "Internal.hpp"
#include <LuminoGraphics/GPU/GraphicsCommandBuffer.hpp>
#include <LuminoGraphicsRHI/GraphicsExtension.hpp>
#include "ExtensionRenderFeature.hpp"

namespace ln {
namespace detail {

//==============================================================================
// ExtensionRenderFeature

ExtensionRenderFeature::ExtensionRenderFeature()
    : m_precondition(NativeGraphicsExtensionRenderPassPreCondition::EnsureInside)
{
}

void ExtensionRenderFeature::init(RenderingManager* manager)
{
	RenderFeature::init();
}

RequestBatchResult ExtensionRenderFeature::invoke(GraphicsCommandList* context, detail::RenderFeatureBatchList* batchList, INativeGraphicsExtension* extension) {
#ifdef LN_RLI_BATCH
    LN_NOTIMPLEMENTED();
    return RequestBatchResult::Submitted;
#else
    bool first = (m_batchData.count == 0);
    m_extensions.push_back(extension);
    m_batchData.count++;

    // TODO: とりいそぎ
    extension->graphicsContext = context;
    extension->renderTarget = batchList->renderTarget;
    extension->depthBuffer = batchList->depthBuffer;

    if (first) {
        m_precondition = extension->getRenderPassPreCondition();
        return RequestBatchResult::Staging;
    }
    else if (m_precondition != extension->getRenderPassPreCondition()) {
        submitBatch(context, batchList);
        return RequestBatchResult::Submitted;
    }
    else {
        return RequestBatchResult::Staging;
    }
#endif
}

void ExtensionRenderFeature::beginRendering()
{
    m_extensions.clear();
    m_batchData.offset = 0;
    m_batchData.count = 0;
}

void ExtensionRenderFeature::submitBatch(GraphicsCommandList* context, detail::RenderFeatureBatchList* batchList) {
#ifdef LN_RLI_BATCH
    LN_UNREACHABLE();
#else
    auto batch = batchList->addNewBatch<Batch>(this);
    batch->ensureRenderPassOutside = (m_precondition == NativeGraphicsExtensionRenderPassPreCondition::EnsureOutside);
    batch->data = m_batchData;

    m_batchData.offset = m_batchData.offset + m_batchData.count;
    m_batchData.count = 0;
#endif
}

void ExtensionRenderFeature::renderBatch(GraphicsCommandList* context, RenderFeatureBatch* batch) {
    auto localBatch = static_cast<Batch*>(batch);
    for (int i = 0; i < localBatch->data.count; i++) {
        context->drawExtension(m_extensions[localBatch->data.offset + i]);
    }
}

} // namespace detail
} // namespace ln

