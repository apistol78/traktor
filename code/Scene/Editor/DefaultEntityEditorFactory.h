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
	T_RTTI_CLASS(DefaultEntityEditorFactory)

public:
	virtual const TypeSet getEntityDataTypes() const;

	virtual IEntityEditor* createEntityEditor(SceneEditorContext* context, const Type& entityDataType) const;
};

	}
}

#endif	// traktor_scene_DefaultEntityEditorFactory_H
