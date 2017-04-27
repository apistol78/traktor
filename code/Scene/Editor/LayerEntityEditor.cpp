/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/LayerEntityEditor.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.LayerEntityEditor", LayerEntityEditor, IEntityEditor)

LayerEntityEditor::LayerEntityEditor(world::LayerEntityData* entityData)
:	m_entityData(entityData)
{
}

bool LayerEntityEditor::isPickable() const
{
	return false;
}

bool LayerEntityEditor::isGroup() const
{
	return true;
}

bool LayerEntityEditor::isChildrenPrivate() const
{
	return false;
}

bool LayerEntityEditor::addChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	T_ASSERT (childEntityData);

	m_entityData->addEntityData(childEntityData);
	return true;
}

bool LayerEntityEditor::removeChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();
	T_ASSERT (childEntityData);

	m_entityData->removeEntityData(childEntityData);
	return true;
}

bool LayerEntityEditor::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	return false;
}

bool LayerEntityEditor::queryFrustum(const Frustum& worldFrustum) const
{
	return false;
}

void LayerEntityEditor::entitySelected(bool selected)
{
}

bool LayerEntityEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void LayerEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
}

bool LayerEntityEditor::getStatusText(std::wstring& outStatusText) const
{
	return false;
}

	}
}
