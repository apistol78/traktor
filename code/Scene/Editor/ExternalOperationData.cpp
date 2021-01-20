#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Scene/Editor/ExternalOperationData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ExternalOperationData", 0, ExternalOperationData, ISerializable)

void ExternalOperationData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"externalDataId", m_externalDataId);
}

	}
}
