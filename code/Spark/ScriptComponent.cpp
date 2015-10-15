#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/Context.h"
#include "Spark/ScriptComponent.h"
#include "Spark/ScriptComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ScriptComponent", 0, ScriptComponent, IComponent)

Ref< IComponentInstance > ScriptComponent::createInstance(const Context* context, SpriteInstance* owner) const
{
	resource::Proxy< IRuntimeClass > clazz;
	if (context->getResourceManager()->bind(m_class, clazz))
		return new ScriptComponentInstance(owner, clazz);
	else
		return 0;
}

void ScriptComponent::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
}

	}
}
