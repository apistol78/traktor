#ifndef traktor_animation_AnimationEntityEditorFactory_H
#define traktor_animation_AnimationEntityEditorFactory_H

#include "Scene/Editor/IEntityEditorFactory.h"

namespace traktor
{
	namespace animation
	{

class AnimationEntityEditorFactory : public scene::IEntityEditorFactory
{
	T_RTTI_CLASS(AnimationEntityEditorFactory)

public:
	virtual const TypeSet getEntityDataTypes() const;

	virtual Ref< scene::IEntityEditor > createEntityEditor(scene::SceneEditorContext* context, const Type& entityDataType) const;
};

	}
}

#endif	// traktor_animation_AnimationEntityEditorFactory_H
