#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObjectRelay", ActionObjectRelay, IActionObjectRelay)

void ActionObjectRelay::setAsObject(ActionObject* asObject)
{
	T_ASSERT (m_asObject == 0);
	m_asObject = asObject;
}

ActionObject* ActionObjectRelay::getAsObject(ActionContext* context)
{
	if (!m_asObject)
	{
		m_asObject = new ActionObject(context, m_prototype, this);
		T_FATAL_ASSERT_M (m_asObject, L"Unable to create AS object");
	}
	return m_asObject;
}

bool ActionObjectRelay::setMember(ActionContext* context, const std::string& memberName, const ActionValue& memberValue)
{
	return false;
}

bool ActionObjectRelay::getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue)
{
	return false;
}

void ActionObjectRelay::trace(const IVisitor& visitor) const
{
	visitor(m_asObject);
}

void ActionObjectRelay::dereference()
{
	m_asObject = 0;
}

ActionObjectRelay::ActionObjectRelay(const char* const prototype)
:	m_prototype(prototype)
{
}

	}
}
