#include "Core/RefArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionObjectCyclic.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObject", ActionObject, Object)

static int32_t s_objectCount = 0;

ActionObject::ActionObject()
:	m_readOnly(false)
,	m_traceColor(TcBlack)
,	m_traceBuffered(false)
,	m_traceRefCount(0)
{
	setMember("__proto__", ActionValue("Object"));
	++s_objectCount;
}

ActionObject::ActionObject(const std::string& prototypeName)
:	m_readOnly(false)
,	m_traceColor(TcBlack)
,	m_traceBuffered(false)
,	m_traceRefCount(0)
{
	setMember("__proto__", ActionValue(prototypeName));
	++s_objectCount;
}

ActionObject::ActionObject(ActionObject* prototype)
:	m_readOnly(false)
,	m_traceColor(TcBlack)
,	m_traceBuffered(false)
,	m_traceRefCount(0)
{
	setMember("__proto__", ActionValue(prototype));
	++s_objectCount;
}

ActionObject::~ActionObject()
{
	--s_objectCount;
}

void ActionObject::release(void* owner) const
{
	if (getReferenceCount() <= 1)
	{
		m_traceColor = TcBlack;
		if (m_traceBuffered)
		{
			m_traceBuffered = false;
			ActionObjectCyclic::getInstance().removeCandidate(const_cast< ActionObject* >(this));
		}
	}
	else
	{
		if (m_traceColor != TcPurple)
		{
			m_traceColor = TcPurple;
			if (!m_traceBuffered)
			{
				m_traceBuffered = true;
				ActionObjectCyclic::getInstance().addCandidate(const_cast< ActionObject* >(this));
			}
		}
	}
	Object::release(owner);
}

void ActionObject::addInterface(ActionObject* intrface)
{
	T_ASSERT (!m_readOnly);
}

ActionObject* ActionObject::getPrototype(ActionContext* context)
{
	Ref< ActionObject > classObject;
	ActionValue protoValue;

	if (getLocalMember("__proto__", protoValue))
	{
		if (protoValue.isObject())
			return protoValue.getObject();
		else if (protoValue.isString())
			classObject = context->lookupClass(protoValue.getString());
	}

	if (!classObject)
	{
		// No such class exist; default to plain "Object".
		if (!(classObject = context->lookupClass("Object")))
			return 0;
	}

	m_members["__proto__"] = ActionValue(classObject);
	return classObject;
}

void ActionObject::setMember(const std::string& memberName, const ActionValue& memberValue)
{
	T_ASSERT (!m_readOnly);
	m_members[memberName] = memberValue;
}

bool ActionObject::getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue)
{
	if (getLocalMember(memberName, outMemberValue))
		return true;

	// Not a local member; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);

			while (prototype)
			{
				if (prototype->getLocalMember(memberName, outMemberValue))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalMember(memberName, outMemberValue))
				return true;
		}
	}

	return false;
}

bool ActionObject::deleteMember(const std::string& memberName)
{
	T_ASSERT (!m_readOnly);

	member_map_t::iterator i = m_members.find(memberName);
	if (i == m_members.end())
		return false;

	m_members.erase(i);
	return true;
}

void ActionObject::deleteAllMembers()
{
	m_members.clear();
}

void ActionObject::addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet)
{
	T_ASSERT (!m_readOnly);
	m_properties[propertyName] = std::make_pair(propertyGet, propertySet);
}

bool ActionObject::getPropertyGet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertyGet)
{
	if (getLocalPropertyGet(propertyName, outPropertyGet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);

			while (prototype)
			{
				if (prototype->getLocalPropertyGet(propertyName, outPropertyGet))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalPropertyGet(propertyName, outPropertyGet))
				return true;
		}
	}

	return false;
}

bool ActionObject::getPropertySet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertySet)
{
	T_ASSERT (!m_readOnly);

	if (getLocalPropertySet(propertyName, outPropertySet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		Ref< ActionObject > prototype = getPrototype(context);
		if (prototype)
		{
			T_ASSERT (prototype != this);
			while (prototype)
			{
				if (prototype->getLocalPropertySet(propertyName, outPropertySet))
					return true;

				Ref< ActionObject > parentPrototype = prototype->getPrototype(context);
				prototype = (parentPrototype != prototype) ? parentPrototype : 0;
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember("prototype", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			if (prototype->getLocalPropertySet(propertyName, outPropertySet))
				return true;
		}
	}

	return false;
}

const ActionObject::property_map_t& ActionObject::getProperties() const
{
	return m_properties;
}

void ActionObject::deleteAllProperties()
{
	m_properties.clear();
}

avm_number_t ActionObject::valueOf() const
{
	return 0.0;
}

ActionValue ActionObject::toString() const
{
	StringOutputStream ss;
	ss << L"object (" << type_name(this) << L")";
	return ActionValue(ss.str());
}

void ActionObject::setReadOnly()
{
	m_readOnly = true;
}

bool ActionObject::getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	if (i == m_members.end())
		return false;

	outMemberValue = i->second;
	return true;
}

bool ActionObject::hasOwnProperty(const std::string& propertyName) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	return i != m_properties.end();
}

bool ActionObject::getLocalPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertyGet = i->second.first;
	return true;
}

bool ActionObject::getLocalPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertySet = i->second.second;
	return true;
}

void ActionObject::trace(const IVisitor& visitor) const
{
	for (member_map_t::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (i->second.isObject())
			visitor(i->second.getObject());
	}
	for (property_map_t::const_iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		if (i->second.first)
			visitor(i->second.first);
		if (i->second.second)
			visitor(i->second.second);
	}
}

void ActionObject::dereference()
{
	m_members.clear();
	m_properties.clear();
}

void ActionObject::traceMarkGray()
{
	if (m_traceColor != TcGray)
	{
		m_traceColor = TcGray;
		m_traceRefCount = getReferenceCount();
		trace(MarkGrayVisitor());
	}
}

void ActionObject::traceScan()
{
	if (m_traceColor == TcGray)
	{
		if (m_traceRefCount > 0)
			traceScanBlack();
		else
		{
			m_traceColor = TcWhite;
			trace(ScanVisitor());
		}
	}
}

void ActionObject::traceScanBlack()
{
	m_traceColor = TcBlack;
	trace(ScanBlackVisitor());
}

void ActionObject::traceCollectWhite()
{
	if (m_traceColor == TcWhite)
	{
		T_ASSERT (m_traceRefCount == 0);
		T_EXCEPTION_GUARD_BEGIN;

		ActionObject::addRef(0);
		dereference();
		ActionObject::release(0);

		T_EXCEPTION_GUARD_END;
	}
}

void ActionObject::MarkGrayVisitor::operator () (ActionObject* memberObject) const
{
	if (memberObject)
	{
		memberObject->traceMarkGray();
		memberObject->m_traceRefCount--;
	}
}

void ActionObject::ScanVisitor::operator () (ActionObject* memberObject) const
{
	if (memberObject)
		memberObject->traceScan();
}

void ActionObject::ScanBlackVisitor::operator () (ActionObject* memberObject) const
{
	if (memberObject)
	{
		memberObject->m_traceRefCount++;
		if (memberObject->m_traceColor != TcBlack)
			memberObject->traceScanBlack();
	}
}

	}
}
