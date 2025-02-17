﻿
#include "Internal.hpp"
#include <LuminoGraphics/Mesh/MeshModel.hpp>
#include <LuminoGraphics/Rendering/Material.hpp>
#include <LuminoEngine/Tilemap/Voxel.hpp>

namespace ln {

//==============================================================================
// VoxelmapModel

LN_OBJECT_IMPLEMENT(VoxelmapModel, Object) {}

VoxelmapModel::VoxelmapModel()
{
}

void VoxelmapModel::init()
{
    Object::init();
    resizeMap(16, 16, 16);
    m_box = MeshModel::load(_TT("C:/Proj/LN/PrivateProjects/HC0/Assets/AutoVolume1.glb"));
    m_mapBlocksMesh = makeObject_deprecated<MeshModel>();
}

void VoxelmapModel::resizeMap(int width, int height, int depth)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_mapData.resize(m_width * m_height * m_depth);
}

void VoxelmapModel::render(RenderingContext* context)
{
    for (const auto& node : m_box->meshNodes()) {
        if (node->meshContainerIndex() >= 0) {
            context->setTransfrom(m_box->nodeGlobalTransform(node->index()));


            const auto& meshContainer = m_box->meshContainers()[node->meshContainerIndex()];

            for (const auto& mesh : meshContainer->meshPrimitives()) {
                for (int iSection = 0; iSection < mesh->sections().size(); iSection++) {
                    context->setMaterial(m_box->materials()[mesh->sections()[iSection].materialIndex]);
                    context->drawMesh(mesh, iSection);
                }
            }

        }
    }

}

} // namespace ln




namespace ln {

//==============================================================================
// VoxelmapComponent

LN_OBJECT_IMPLEMENT(VoxelmapComponent, VisualComponent) {}

VoxelmapComponent::VoxelmapComponent()
{
}

void VoxelmapComponent::init()
{
    VisualComponent::init();
    m_model = makeObject_deprecated<VoxelmapModel>();
}

void VoxelmapComponent::onRender(RenderingContext* context)
{
    m_model->render(context);
}

} // namespace ln
