#ifndef traktor_amalgam_PrefabEntityEditorFactory_H
#define traktor_amalgam_PrefabEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace amalgam
	{

class PrefabEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const override final;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter) const override final;
};

	}
}

#endif	// traktor_amalgam_PrefabEntityEditorFactory_H
