#ifndef traktor_scene_LayerEntityEditor_H
#define traktor_scene_LayerEntityEditor_H

#include "Scene/Editor/IEntityEditor.h"

namespace traktor
{
	namespace scene
	{

class LayerEntityData;

class LayerEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	LayerEntityEditor(LayerEntityData* entityData);

	virtual bool isPickable() const;

	virtual bool isGroup() const;

	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const;

	virtual bool queryFrustum(const Frustum& worldFrustum) const;

	virtual void entitySelected(bool selected);

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;

	virtual bool getStatusText(std::wstring& outStatusText) const;

private:
	LayerEntityData* m_entityData;
};

	}
}

#endif	// traktor_scene_LayerEntityEditor_H
