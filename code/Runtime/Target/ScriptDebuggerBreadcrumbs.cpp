#include "Runtime/Target/ScriptDebuggerBreadcrumbs.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerBreadcrumbs", 0, ScriptDebuggerBreadcrumbs, ISerializable)

ScriptDebuggerBreadcrumbs::ScriptDebuggerBreadcrumbs(const AlignedVector< uint32_t >& breadcrumbs)
:	m_breadcrumbs(breadcrumbs)
{
}

void ScriptDebuggerBreadcrumbs::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< uint32_t >(L"breadcrumbs", m_breadcrumbs);
}

	}
}
