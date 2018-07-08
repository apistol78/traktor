#include "Core/Class/Any.h"
#include "Core/Class/PolymorphicDispatch.h"
#include "Core/Io/OutputStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PolymorphicDispatch", PolymorphicDispatch, IRuntimeDispatch)

void PolymorphicDispatch::set(uint32_t argc, const IRuntimeDispatch* dispatch)
{
	if (m_dispatches.size() < argc + 1)
		m_dispatches.resize(argc + 1);
	m_dispatches[argc] = dispatch;
}

void PolymorphicDispatch::signature(OutputStream& ss) const
{
	bool first = true;
	for (RefArray< const IRuntimeDispatch >::const_iterator i = m_dispatches.begin(); i != m_dispatches.end(); ++i)
	{
		if (*i)
		{
			if (!first)
				ss << L";";
			(*i)->signature(ss);
			first = false;
		}
	}
}

Any PolymorphicDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	if (argc < m_dispatches.size() && m_dispatches[argc])
		return m_dispatches[argc]->invoke(self, argc, argv);
	else
		return Any();
}

}
