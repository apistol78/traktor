#include "Core/Class/Any.h"
#include "Core/Class/PolymorphicDispatch.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.PolymorphicDispatch", PolymorphicDispatch, IRuntimeDispatch)

void PolymorphicDispatch::set(uint32_t argc, const IRuntimeDispatch* dispatch)
{
	if (m_dispatches.size() < argc + 1)
		m_dispatches.resize(argc + 1);
	m_dispatches[argc] = dispatch;
}

std::wstring PolymorphicDispatch::signature() const
{
	std::wstring s;
	for (auto dispatch : m_dispatches)
	{
		if (dispatch)
		{
			if (!s.empty())
				s += L";";
			s += dispatch->signature();
		}
	}
	return s;
}

Any PolymorphicDispatch::invoke(ITypedObject* self, uint32_t argc, const Any* argv) const
{
	if (argc < m_dispatches.size() && m_dispatches[argc])
		return m_dispatches[argc]->invoke(self, argc, argv);
	else
		return Any();
}

}
