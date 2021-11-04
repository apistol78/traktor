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

#if defined(T_NEED_RUNTIME_SIGNATURE)
void PolymorphicDispatch::signature(OutputStream& os) const
{
	bool first = true;
	for (auto dispatch : m_dispatches)
	{
		if (dispatch)
		{
			if (first)
				os << L";";
			dispatch->signature(os);
			first = false;
		}
	}
}
#endif

Any PolymorphicDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	if (argc < m_dispatches.size() && m_dispatches[argc])
		return m_dispatches[argc]->invoke(self, argc, argv);
	else
		return Any();
}

}
