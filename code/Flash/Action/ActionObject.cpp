#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionObject", ActionObject, Object)

ActionObject::ActionObject()
:	m_readOnly(false)
{
}

ActionObject::ActionObject(ActionObject* classObject)
:	m_readOnly(false)
{
	ActionValue classPrototype;
	if (classObject->getLocalMember(L"prototype", classPrototype))
		setMember(L"__proto__", classPrototype);
}

void ActionObject::addInterface(ActionObject* intrface)
{
	T_ASSERT (!m_readOnly);
}

void ActionObject::setMember(const std::wstring& memberName, const ActionValue& memberValue)
{
	T_ASSERT (!m_readOnly);
	m_members[memberName] = memberValue;
}

bool ActionObject::getMember(const std::wstring& memberName, ActionValue& outMemberValue) const
{
	if (getLocalMember(memberName, outMemberValue))
		return true;

	// Not a local member; try to get from our __proto__ reference.
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"__proto__", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			while (prototype)
			{
				if (prototype->getLocalMember(memberName, outMemberValue))
					return true;

				if (!prototype->getLocalMember(L"__proto__", prototypeValue))
					break;

				prototype = prototypeValue.getObjectSafe();
				T_ASSERT (prototype != this);
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"prototype", prototypeValue))
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

bool ActionObject::deleteMember(const std::wstring& memberName)
{
	T_ASSERT (!m_readOnly);

	member_map_t::iterator i = m_members.find(memberName);
	if (i == m_members.end())
		return false;

	m_members.erase(i);
	return true;
}

void ActionObject::addProperty(const std::wstring& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet)
{
	T_ASSERT (!m_readOnly);
	m_properties[propertyName] = std::make_pair(propertyGet, propertySet);
}

bool ActionObject::getPropertyGet(const std::wstring& propertyName, Ref< ActionFunction >& outPropertyGet) const
{
	if (getLocalPropertyGet(propertyName, outPropertyGet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"__proto__", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			while (prototype)
			{
				if (prototype->getLocalPropertyGet(propertyName, outPropertyGet))
					return true;

				if (!prototype->getLocalMember(L"__proto__", prototypeValue))
					break;

				prototype = prototypeValue.getObjectSafe();
				T_ASSERT (prototype != this);
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"prototype", prototypeValue))
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

bool ActionObject::getPropertySet(const std::wstring& propertyName, Ref< ActionFunction >& outPropertySet) const
{
	T_ASSERT (!m_readOnly);

	if (getLocalPropertySet(propertyName, outPropertySet))
		return true;

	// Not a local property; try to get from our __proto__ reference.
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"__proto__", prototypeValue))
		{
			Ref< ActionObject > prototype = prototypeValue.getObjectSafe();
			T_ASSERT (prototype != this);
			T_ASSERT (prototype);

			while (prototype)
			{
				if (prototype->getLocalPropertySet(propertyName, outPropertySet))
					return true;

				if (!prototype->getLocalMember(L"__proto__", prototypeValue))
					break;

				prototype = prototypeValue.getObjectSafe();
				T_ASSERT (prototype != this);
			}
		}
	}

	// Not available through __proto__ chain; assume class object and try from "prototype".
	{
		ActionValue prototypeValue;
		if (getLocalMember(L"prototype", prototypeValue))
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

double ActionObject::valueOf() const
{
	return 0.0;
}

std::wstring ActionObject::toString() const
{
	StringOutputStream ss; ss << L"object (" << uint32_t(this) << L" " << type_name(this) << L")";
	return ss.str();
}

void ActionObject::setReadOnly()
{
	m_readOnly = true;
}

bool ActionObject::getLocalMember(const std::wstring& memberName, ActionValue& outMemberValue) const
{
	member_map_t::const_iterator i = m_members.find(memberName);
	if (i == m_members.end())
		return false;
	
	outMemberValue = i->second;
	return true;
}

bool ActionObject::hasOwnProperty(const std::wstring& propertyName) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	return i != m_properties.end();
}

bool ActionObject::getLocalPropertyGet(const std::wstring& propertyName, Ref< ActionFunction >& outPropertyGet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertyGet = i->second.first;
	return true;
}

bool ActionObject::getLocalPropertySet(const std::wstring& propertyName, Ref< ActionFunction >& outPropertySet) const
{
	property_map_t::const_iterator i = m_properties.find(propertyName);
	if (i == m_properties.end())
		return false;

	outPropertySet = i->second.second;
	return true;
}

	}
}
