#ifndef traktor_illuminate_IlluminateEntityEditorFactory_H
#define traktor_illuminate_IlluminateEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

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

class T_DLLCLASS IlluminateEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_illuminate_IlluminateEntityEditorFactory_H
