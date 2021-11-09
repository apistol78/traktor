#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyGroup.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/Entity/ScriptComponent.h"
#include "World/Entity/ScriptComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ScriptComponentData", 2, ScriptComponentData, IEntityComponentData)

ScriptComponentData::ScriptComponentData(const resource::Id< IRuntimeClass >& _class)
:	m_class(_class)
{
}

Ref< ScriptComponent > ScriptComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< IRuntimeClass > clazz;
	if (resourceManager->bind(m_class, clazz))
		return new ScriptComponent(clazz, m_properties);
	else
		return nullptr;
}

void ScriptComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void ScriptComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"class", m_class);
	if (s.getVersion< ScriptComponentData >() >= 2)
		s >> MemberRef< const PropertyGroup >(L"properties", m_properties);
	if (s.getVersion< ScriptComponentData >() >= 1)
		s >> Member< bool >(L"editorSupport", m_editorSupport);
}

	}
}
