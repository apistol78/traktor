#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
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

CombinedInputSourceData::CombinedInputSourceData(const RefArray< IInputSourceData >& sources)
:	m_sources(sources)
{
}

void CombinedInputSourceData::addSource(IInputSourceData* source)
{
	m_sources.push_back(source);
}

const RefArray< IInputSourceData >& CombinedInputSourceData::getSources() const
{
	return m_sources;
}

Ref< IInputSource > CombinedInputSourceData::createInstance() const
{
	RefArray< IInputSource > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		Ref< IInputSource > source = m_sources[i]->createInstance();
		if (!source)
			return 0;
			
		sources[i] = source;
	}
	return new CombinedInputSource(sources);
}

bool CombinedInputSourceData::serialize(ISerializer& s)
{
	s >> MemberRefArray< IInputSourceData >(L"sources", m_sources);
	return true;
}
	
	}
}
