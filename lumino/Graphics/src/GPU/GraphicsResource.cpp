﻿
#include "Internal.hpp"
#include <LuminoGraphics/GPU/GraphicsResource.hpp>
#include <LuminoGraphics/detail/GraphicsManager.hpp>
#include <LuminoGraphicsRHI/GraphicsDeviceContext.hpp>

namespace ln {

////==============================================================================
//// GraphicsResource
//
//LN_OBJECT_IMPLEMENT(GraphicsResource, Object) {}
//
//GraphicsResource::GraphicsResource()
//	: m_manager(nullptr)
//{
//}
//
//GraphicsResource::~GraphicsResource()
//{
//}
//
//void GraphicsResource::init()
//{
//	if (LN_REQUIRE(!m_manager)) return;
//	m_manager = detail::EngineDomain::graphicsManager();
//	m_manager->addGraphicsResource(this);
//}
//
//void GraphicsResource::onDispose(bool explicitDisposing)
//{
//	if (m_manager) {
//		m_manager->removeGraphicsResource(this);
//		m_manager = nullptr;
//	}
//	Object::onDispose(explicitDisposing);
//}
 namespace detail {
     
void GraphicsResourceInternal::initializeHelper_GraphicsResource(IGraphicsResource* obj, GraphicsManager** manager)
{
    *manager = detail::GraphicsManager::instance();
    (*manager)->addGraphicsResource(obj);
}

void GraphicsResourceInternal::finalizeHelper_GraphicsResource(IGraphicsResource* obj, GraphicsManager** manager)
{
    if (*manager) {
        (*manager)->removeGraphicsResource(obj);
        *manager = nullptr;
    }
}

} // namespace detail
} // namespace ln
