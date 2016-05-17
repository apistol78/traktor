#ifndef traktor_scene_DefaultEntityEditorFactory_H
#define traktor_scene_DefaultEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace scene
	{

/*! \brief
 */
class DefaultEntityEditorFactory : public IEntityEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityDataTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< IEntityEditor > createEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_scene_DefaultEntityEditorFactory_H
