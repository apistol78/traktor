#include "Model/Model.h"
#include "Model/Operations/Clear.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Clear", Clear, IModelOperation)

Clear::Clear(uint32_t clearFlags)
:	m_clearFlags(clearFlags)
{
}

bool Clear::apply(Model& model) const
{
	model.clear(m_clearFlags);
	return true;
}

	}
}
