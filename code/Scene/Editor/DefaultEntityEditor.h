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

class T_DLLCLASS DefaultEntityEditor : public IEntityEditor
{
	T_RTTI_CLASS;

public:
	DefaultEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter);

	virtual bool isGroup() const;

	virtual bool addChildEntity(EntityAdapter* childEntityAdapter) const;

	virtual bool removeChildEntity(EntityAdapter* childEntityAdapter) const;

	virtual bool queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const;

	virtual void entitySelected(bool selected);

	virtual void beginModifier();

	virtual void applyModifier(
		const Matrix44& viewTransform,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		const Vector4& worldDelta,
		int mouseButton
	);

	virtual void endModifier();

	virtual bool handleCommand(const ui::Command& command);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const;

	virtual bool getStatusText(std::wstring& outStatusText) const;

protected:
	SceneEditorContext* getContext() { return m_context; }

	EntityAdapter* getEntityAdapter() { return m_entityAdapter; }

	const SceneEditorContext* getContext() const { return m_context; }

	const EntityAdapter* getEntityAdapter() const { return m_entityAdapter; }

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
