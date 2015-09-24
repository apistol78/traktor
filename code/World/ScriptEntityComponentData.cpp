#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/ScriptEntityComponent.h"
#include "World/ScriptEntityComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ScriptEntityComponentData", 0, ScriptEntityComponentData, IEntityComponentData)

Ref< IEntityComponent > ScriptEntityComponentData::createInstance(Entity* owner, resource::IResourceManager* resourceManager) const
{
	resource::Proxy< IRuntimeClass > clazz;
	if (!resourceManager->bind(m_class, clazz))
		return 0;

	return new ScriptEntityComponent(owner, clazz);
}

void ScriptEntityComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
}

	}
}
