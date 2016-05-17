#ifndef traktor_scene_DefaultEntityEditor_H
#define traktor_scene_DefaultEntityEditor_H

#include "Core/Math/Color4ub.h"
#include "Core/Math/Transform.h"
#include "Scene/Editor/IEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class SceneEditorContext;

class T_DLLCLASS DefaultEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	DefaultEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter);

	virtual bool isPickable() const T_OVERRIDE;

	virtual bool isGroup() const T_OVERRIDE;

	virtual bool isChildrenPrivate() const T_OVERRIDE;

	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const T_OVERRIDE;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const T_OVERRIDE;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const T_OVERRIDE;

	virtual bool queryFrustum(const Frustum& worldFrustum) const T_OVERRIDE;

	virtual void entitySelected(bool selected) T_OVERRIDE;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE;

	virtual bool getStatusText(std::wstring& outStatusText) const T_OVERRIDE;

protected:
	SceneEditorContext* getContext() { return m_context; }

	EntityAdapter* getEntityAdapter() { return m_entityAdapter; }

	const SceneEditorContext* getContext() const { return m_context; }

	const EntityAdapter* getEntityAdapter() const { return m_entityAdapter; }

	bool inModify() const { return m_inModify; }

private:
	SceneEditorContext* m_context;
	EntityAdapter* m_entityAdapter;
	bool m_inModify;
	Transform m_modifyTransform;
	Color4ub m_colorBoundingBox;
	Color4ub m_colorBoundingBoxSel;
	Color4ub m_colorBoundingBoxFaceSel;
	Color4ub m_colorSnap;

	void updateSettings();
};

	}
}

#endif	// traktor_scene_DefaultEntityEditor_H
