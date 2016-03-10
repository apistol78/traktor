#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObjectRelay", ActionObjectRelay, Collectable)

void ActionObjectRelay::setAsObject(ActionObject* asObject)
{
	T_ASSERT (asObject == 0 || m_asObject == 0);
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

bool ActionObjectRelay::enumerateMembers(std::vector< uint32_t >& outMemberNames) const
{
	return false;
}

bool ActionObjectRelay::setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue)
{
	return false;
}

bool ActionObjectRelay::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	return false;
}

void ActionObjectRelay::trace(visitor_t visitor) const
{
	visitor(m_asObject);
}

void ActionObjectRelay::dereference()
{
	if (m_asObject)
	{
		m_asObject->dereference();
		m_asObject = 0;
	}
}

ActionObjectRelay::ActionObjectRelay(const char* const prototype)
:	m_prototype(prototype)
{
}

	}
}
