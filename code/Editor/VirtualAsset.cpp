#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Editor/VirtualAsset.h"
#include "Editor/VirtualPlaceholder.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.VirtualAsset", 0, VirtualAsset, ISerializable)

const Guid& VirtualAsset::getSourceInstance() const
{
	return m_sourceInstance;
}

const Guid& VirtualAsset::getPlaceholderInstance() const
{
	return m_placeholderInstance;
}

void VirtualAsset::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"sourceInstance", m_sourceInstance);
	s >> Member< Guid >(L"placeholderInstance", m_placeholderInstance, AttributeType(type_of< VirtualPlaceholder >()));
}

	}
}
