#pragma once

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

	virtual bool isPickable() const override final;

	virtual bool isGroup() const override final;

	virtual bool isChildrenPrivate() const override final;

	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const override final;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const override final;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const override final;

	virtual bool queryFrustum(const Frustum& worldFrustum) const override final;

	virtual void entitySelected(bool selected) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

	virtual bool getStatusText(std::wstring& outStatusText) const override final;

private:
	world::LayerEntityData* m_entityData;
};

	}
}

