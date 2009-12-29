#include "Flash/Action/Avm1/ActionSuper.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionSuper", ActionSuper, ActionFunction)

ActionSuper::ActionSuper(ActionObject* object)
:	ActionFunction(L"super")
,	m_object(object)
{
	ActionValue memberValue;

	// __proto__
	m_object->getMember(L"__proto__", memberValue);
	Ref< ActionObject > prototype = memberValue.getObject();

	// __proto__.__proto__
	if (prototype->getLocalMember(L"__proto__", memberValue))
		m_superPrototype = memberValue.getObject();

	// __proto__.__constructor__
	if (prototype->getLocalMember(L"__constructor__", memberValue))
		m_superClass = memberValue.getObject();
	else
		m_superClass = dynamic_type_cast< ActionFunction* >(m_superPrototype);

	// Set super prototype as our __proto__.
	setMember(L"__proto__", ActionValue::fromObject(m_superPrototype));
	setReadOnly();

	if (m_superClass)
		log::debug << L"ActionSuper; using class " << checked_type_cast< ActionFunction* >(m_superClass)->getName() << Endl;
	else
		log::debug << L"ActionSuper; no constructor" << Endl;
}

ActionValue ActionSuper::call(const IActionVM* vm, ActionFrame* callerFrame, ActionObject* self)
{
	Ref< ActionFunction > superCtor = dynamic_type_cast< ActionFunction* >(m_superClass);
	return superCtor ? superCtor->call(vm, callerFrame, self) : ActionValue();
}

	}
}
