#include "Scene/Editor/DefaultComponentEditor.h"
#include "Scene/Editor/DefaultComponentEditorFactory.h"
#include "World/IEntityComponentData.h"

namespace traktor
{
	namespace scene
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultComponentEditorFactory", DefaultComponentEditorFactory, IComponentEditorFactory)	

const TypeInfoSet DefaultComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< world::IEntityComponentData >();
	//	world::CameraComponentData,
	//	world::LightComponentData,
	//	world::VolumeComponentData
	//>();
}

Ref< IComponentEditor > DefaultComponentEditorFactory::createComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new DefaultComponentEditor(context, entityAdapter, componentData);
}

	}
}
