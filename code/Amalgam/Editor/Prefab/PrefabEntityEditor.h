#ifndef traktor_amalgam_PrefabEntityEditor_H
#define traktor_amalgam_PrefabEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS PrefabEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	PrefabEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool isPickable() const T_OVERRIDE T_FINAL;

	virtual bool isGroup() const T_OVERRIDE T_FINAL;

	virtual bool addChildEntity(scene::EntityAdapter* childEntityAdapter) const T_OVERRIDE T_FINAL;

	virtual bool removeChildEntity(scene::EntityAdapter* childEntityAdapter) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_PrefabEntityEditor_H
