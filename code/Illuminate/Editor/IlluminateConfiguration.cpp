#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.illuminate.IlluminateConfiguration", 0, IlluminateConfiguration, ISerializable)

IlluminateConfiguration::IlluminateConfiguration()
:	m_traceDirect(true)
,	m_traceIndirect(true)
{
}

void IlluminateConfiguration::serialize(ISerializer& s)
{
	s >> Member< bool >(L"traceDirect", m_traceDirect);
	s >> Member< bool >(L"traceIndirect", m_traceIndirect);
}

	}
}