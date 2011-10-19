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

ActionObject* ActionObjectRelay::getAsObject()
{
	if (!m_asObject)
	{
		m_asObject = new ActionObject(m_prototype, this);
		T_FATAL_ASSERT_M (m_asObject, L"Unable to create AS object");
	}
	return m_asObject;
}

bool ActionObjectRelay::setMember(const std::string& memberName, const ActionValue& memberValue)
{
	return false;
}

bool ActionObjectRelay::getMember(const std::string& memberName, ActionValue& outMemberValue)
{
	return false;
}

ActionValue ActionObjectRelay::toString() const
{
	return ActionValue("[object Object]");
}

void ActionObjectRelay::trace(const IVisitor& visitor) const
{
	visitor(m_asObject);
}

void ActionObjectRelay::dereference()
{
	m_asObject = 0;
}

ActionObjectRelay::ActionObjectRelay(const std::string& prototype)
:	m_prototype(prototype)
{
}

	}
}
