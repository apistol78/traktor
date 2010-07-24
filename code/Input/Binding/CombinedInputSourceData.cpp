#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/CombinedInputSource.h"
#include "Input/Binding/CombinedInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.CombinedInputSourceData", 0, CombinedInputSourceData, IInputSourceData)

CombinedInputSourceData::CombinedInputSourceData()
{
}

CombinedInputSourceData::CombinedInputSourceData(
	IInputSourceData* source1,
	IInputSourceData* source2
)
:	m_source1(source1)
,	m_source2(source1)
{
}

void CombinedInputSourceData::setSource1(IInputSourceData* source1)
{
	m_source1 = source1;
}

IInputSourceData* CombinedInputSourceData::getSource1() const
{
	return m_source1;
}

void CombinedInputSourceData::setSource2(IInputSourceData* source2)
{
	m_source2 = source2;
}

IInputSourceData* CombinedInputSourceData::getSource2() const
{
	return m_source2;
}

Ref< IInputSource > CombinedInputSourceData::createInstance() const
{
	Ref< IInputSource > source1 = m_source1 ? m_source1->createInstance() : 0;
	Ref< IInputSource > source2 = m_source2 ? m_source2->createInstance() : 0;
	return new CombinedInputSource(source1, source2);
}

bool CombinedInputSourceData::serialize(ISerializer& s)
{
	s >> MemberRef< IInputSourceData >(L"source1", m_source1);
	s >> MemberRef< IInputSourceData >(L"source2", m_source2);
	return true;
}
	
	}
}
