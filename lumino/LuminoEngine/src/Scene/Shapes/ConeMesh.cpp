﻿
#include "Internal.hpp"
#include <LuminoGraphics/Rendering/Material.hpp>
#include <LuminoGraphics/Mesh/MeshModel.hpp>
#include <LuminoGraphics/Mesh/MeshModelFactory.hpp>
#include <LuminoEngine/Scene/Shapes/ConeMesh.hpp>

namespace ln {

//==============================================================================
// ConeMesh

ConeMesh::ConeMesh() = default;

ConeMesh::~ConeMesh() = default;

bool ConeMesh::init(float radius, float height, float segments, Material* material)
{
	auto model = detail::MeshModelFactory::createCone(radius, height, segments, material);
	if (!StaticMesh::init(model)) return false;
	return true;
}

//==============================================================================
// ConeMesh::BuilderDetails

ConeMesh::BuilderDetails::BuilderDetails()
	: radius(0.5f)
	, height(1.0f)
	, segments(16)
	, material(nullptr)
{
}

Ref<Object> ConeMesh::BuilderDetails::create() const
{
	auto p = makeObject_deprecated<ConeMesh>(radius, height, segments, material);
	apply(p);
	return p;
}

} // namespace ln

