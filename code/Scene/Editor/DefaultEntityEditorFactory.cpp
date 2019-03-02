#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/DefaultEntityEditorFactory.h"
#include "World/EntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditorFactory", DefaultEntityEditorFactory, IEntityEditorFactory)

const TypeInfoSet DefaultEntityEditorFactory::getEntityDataTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< world::EntityData >());
	return typeSet;
}

Ref< IEntityEditor > DefaultEntityEditorFactory::createEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter) const
{
	return new DefaultEntityEditor(context, entityAdapter);
}

	}
}
