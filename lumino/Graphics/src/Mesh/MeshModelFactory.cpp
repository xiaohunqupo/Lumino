﻿
#include "Internal.hpp"
#include <LuminoGraphics/Rendering/Material.hpp>
#include <LuminoGraphics/Mesh/MeshModel.hpp>
#include <LuminoGraphics/Mesh/MeshModelFactory.hpp>
#include <LuminoGraphics/Rendering/detail/RenderingManager.hpp>
#include "MeshGeneraters/MeshGenerater.hpp"
#include "MeshGeneraters/CylinderMeshGenerater.hpp"
#include "MeshGeneraters/TeapotMeshGenerater.hpp"

namespace ln {
namespace detail {
	
//==============================================================================
// MeshModelFactory

Ref<MeshModel> MeshModelFactory::createBox(const Vector3& size, Material* material)
{
	RegularBoxMeshFactory g;
	g.setColor(Color::White);
	g.m_size = size;
	return createMesh(&g, material);
}

Ref<MeshModel> MeshModelFactory::createCone(float radius, float height, float segments, Material* material)
{
	ConeMeshFactory g;
	g.init(radius, height, segments);
	g.setColor(Color::White);
	return createMesh(&g, material);
}

Ref<MeshModel> MeshModelFactory::createCylinder(float radiusTop, float radiusBottom, float height, float segments, Material* material)
{
	RegularCylinderMeshGenerater g;
	g.init(radiusTop, radiusBottom, height, segments, 1);
	g.setColor(Color::White);
	return createMesh(&g, material);
}

Ref<MeshModel> MeshModelFactory::createTeapot(Material* material)
{
	TeapotMeshGenerater g;
	g.m_size = 1.0f;
	g.m_tessellation = 8;
	g.setColor(Color::White);
	return createMesh(&g, material);
}

Ref<MeshModel> MeshModelFactory::createMesh(MeshGenerater* factory, Material* material)
{
    Ref<MeshPrimitive> mesh = makeObject_deprecated<MeshPrimitive>(factory->vertexCount(), factory->indexCount());

	MeshGeneraterBuffer buffer(nullptr);
	buffer.setBuffer(static_cast<Vertex*>(mesh->acquireMappedVertexBuffer(InterleavedVertexGroup::Main)), mesh->acquireMappedIndexBuffer(), mesh->indexBufferFormat(), 0);
	buffer.generate(factory);

	mesh->addSection(0, factory->indexCount() / 3, 0, factory->primitiveType());	// TODO: primitiveCount by toporogy

	Ref<MeshModel> model = makeObject_deprecated<MeshModel>();
	MeshNode* node = model->addMeshContainerNode(mesh);
	model->addMaterial(material ? material : RenderingManager::instance()->primitiveMeshDefaultMaterial().get());

	model->calculateBoundingBox();

	return model;
}

} // namespace detail
} // namespace ln

