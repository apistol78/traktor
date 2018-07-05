#include "Core/Class/Any.h"
#include "Core/Class/OperatorDispatch.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OperatorDispatch", OperatorDispatch, IRuntimeDispatch)

void OperatorDispatch::add(const IRuntimeDispatch* dispatch)
{
	m_dispatches.push_back(dispatch);
}

void OperatorDispatch::signature(OutputStream& ss) const
{
}

Any OperatorDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	Any result;
	for (RefArray< const IRuntimeDispatch >::const_iterator i = m_dispatches.begin(); i != m_dispatches.end(); ++i)
	{
		result = (*i)->invoke(self, argc, argv);
		if (!result.isVoid())
			break;
	}
	return result;
}

}
