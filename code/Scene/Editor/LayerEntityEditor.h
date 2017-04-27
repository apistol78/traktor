/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_LayerEntityEditor_H
#define traktor_scene_LayerEntityEditor_H

#include "Scene/Editor/IEntityEditor.h"

namespace traktor
{
	namespace world
	{

class LayerEntityData;

	}

	namespace scene
	{

class LayerEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	LayerEntityEditor(world::LayerEntityData* entityData);

	virtual bool isPickable() const T_OVERRIDE T_FINAL;

	virtual bool isGroup() const T_OVERRIDE T_FINAL;

	virtual bool isChildrenPrivate() const T_OVERRIDE T_FINAL;

	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const T_OVERRIDE T_FINAL;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const T_OVERRIDE T_FINAL;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const T_OVERRIDE T_FINAL;

	virtual bool queryFrustum(const Frustum& worldFrustum) const T_OVERRIDE T_FINAL;

	virtual void entitySelected(bool selected) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

	virtual bool getStatusText(std::wstring& outStatusText) const T_OVERRIDE T_FINAL;

private:
	world::LayerEntityData* m_entityData;
};

	}
}

#endif	// traktor_scene_LayerEntityEditor_H
