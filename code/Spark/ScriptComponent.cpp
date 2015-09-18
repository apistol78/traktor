#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/ScriptComponent.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ScriptComponent", 0, ScriptComponent, IComponent)

Ref< IComponentInstance > ScriptComponent::createInstance(SpriteInstance* owner, resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const
{
	resource::Proxy< IRuntimeClass > clazz;
	if (!resourceManager->bind(m_class, clazz))
		return 0;

	return new ScriptComponentInstance(owner, clazz);
}

void ScriptComponent::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
}

	}
}
