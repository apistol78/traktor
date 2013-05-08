#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InputMappingResource.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingResource", 0, InputMappingResource, ISerializable)

InputMappingResource::InputMappingResource()
{
}

InputMappingResource::InputMappingResource(InputMappingSourceData* sourceData, InputMappingStateData* stateData)
:	m_sourceData(sourceData)
,	m_stateData(stateData)
{
}

void InputMappingResource::serialize(ISerializer& s)
{
	s >> MemberRef< InputMappingSourceData >(L"sourceData", m_sourceData);
	s >> MemberRef< InputMappingStateData >(L"stateData", m_stateData);
}

	}
}
