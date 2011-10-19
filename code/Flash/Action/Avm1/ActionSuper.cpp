#include "Flash/Action/Avm1/ActionSuper.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionSuper", ActionSuper, ActionFunction)

ActionSuper::ActionSuper(ActionContext* context, ActionObject* object)
:	ActionFunction("super")
,	m_object(object)
{
	ActionValue memberValue;

	// __proto__
	Ref< ActionObject > prototype = m_object->getPrototype(context);

	// __proto__.__proto__
	m_superPrototype = prototype->getPrototype(context);

	// __proto__.__ctor__
	if (prototype->getLocalMember("__ctor__", memberValue))
		m_superClass = memberValue.getObject();
	else
		m_superClass = dynamic_type_cast< ActionFunction* >(m_superPrototype);

	// Set super prototype as our __proto__.
	setMember("__proto__", ActionValue(m_superPrototype));
	setReadOnly();
}

ActionValue ActionSuper::call(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	return ActionValue();
}

ActionValue ActionSuper::call(ActionFrame* callerFrame, ActionObject* self)
{
	Ref< ActionFunction > superCtor = dynamic_type_cast< ActionFunction* >(m_superClass);
	return superCtor ? superCtor->call(callerFrame, self) : ActionValue();
}

void ActionSuper::trace(const IVisitor& visitor) const
{
	visitor(m_superClass);
	visitor(m_superPrototype);
	visitor(m_object);
	ActionFunction::trace(visitor);
}

void ActionSuper::dereference()
{
	m_superClass = 0;
	m_superPrototype = 0;
	m_object = 0;
	ActionFunction::dereference();
}

	}
}
