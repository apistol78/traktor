#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spark/Context.h"
#include "Spark/Script/ScriptComponentData.h"
#include "Spark/Script/ScriptComponent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ScriptComponentData", 0, ScriptComponentData, IComponentData)

Ref< IComponent > ScriptComponentData::createInstance(const Context* context, Sprite* owner) const
{
	resource::Proxy< IRuntimeClass > clazz;
	if (context->getResourceManager()->bind(m_class, clazz))
		return new ScriptComponent(owner, clazz);
	else
		return 0;
}

void ScriptComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
}

	}
}
