#ifndef traktor_illuminate_IlluminateEntityEditor_H
#define traktor_illuminate_IlluminateEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	IlluminateEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool isPickable() const;

	virtual bool isGroup() const;

	virtual bool addChildEntity(scene::EntityAdapter* childEntityAdapter) const;

	virtual bool removeChildEntity(scene::EntityAdapter* childEntityAdapter) const;
};

	}
}

#endif	// traktor_illuminate_IlluminateEntityEditor_H
