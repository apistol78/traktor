#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Editor/EditorAttributesComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EditorAttributesComponentData", 0, EditorAttributesComponentData, IEntityComponentData)

void EditorAttributesComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void EditorAttributesComponentData::serialize(ISerializer& s)
{
	s >> Member< bool >(L"visible", visible);
	s >> Member< bool >(L"locked", locked);
	s >> Member< bool >(L"include", include);
	s >> Member< bool >(L"dynamic", dynamic);
}

	}
}
