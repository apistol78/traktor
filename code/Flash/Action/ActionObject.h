#ifndef traktor_flash_ActionObject_H
#define traktor_flash_ActionObject_H

#include <map>
#include <string>
#include "Flash/Collectable.h"
#include "Flash/Action/ActionTypes.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionFunction;
class ActionValue;
class IActionObjectRelay;

/*! \brief ActionScript object.
 * \ingroup Flash
 */
class T_DLLCLASS ActionObject : public Collectable
{
	T_RTTI_CLASS;

public:
	typedef std::map< std::string, ActionValue > member_map_t;
	typedef std::map< std::string, std::pair< Ref< ActionFunction >, Ref< ActionFunction > > > property_map_t;

	ActionObject(ActionContext* context, IActionObjectRelay* relay = 0);

	explicit ActionObject(ActionContext* context, const std::string& prototypeName, IActionObjectRelay* relay = 0);

	explicit ActionObject(ActionContext* context, ActionObject* prototype, IActionObjectRelay* relay = 0);

	virtual void addInterface(ActionObject* intrface);

	void set__proto__(ActionObject* prototype);

	virtual ActionObject* get__proto__();

	virtual void setMember(const std::string& memberName, const ActionValue& memberValue);

	virtual bool getMember(const std::string& memberName, ActionValue& outMemberValue);

	virtual bool deleteMember(const std::string& memberName);

	virtual void deleteAllMembers();

	virtual void addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet);

	virtual bool getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet);

	virtual bool getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet);

	virtual const property_map_t& getProperties() const;

	virtual void deleteAllProperties();

	virtual ActionValue valueOf();

	virtual ActionValue toString();

	void setReadOnly();

	bool hasOwnMember(const std::string& memberName) const;

	bool getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const;

	bool hasOwnProperty(const std::string& propertyName) const;

	bool getLocalPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet) const;

	bool getLocalPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet) const;

	void setRelay(IActionObjectRelay* relay);

	IActionObjectRelay* getRelay() const { return m_relay; }

	template < typename RelayedType >
	RelayedType* getRelay() const { return dynamic_type_cast< RelayedType* >(getRelay()); }

	ActionContext* getContext() const { return m_context; }

	const member_map_t& getLocalMembers() const { return m_members; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

	void setOverrideRelay(IActionObjectRelay* relay);

private:
	ActionContext* m_context;
	bool m_readOnly;
	mutable member_map_t m_members;
	property_map_t m_properties;
	Ref< ActionObject > m__proto__;		//!< Cached "__proto__" member value.
	Ref< ActionObject > m_prototype;	//!< Cached "prototype" member value.
	Ref< IActionObjectRelay > m_relay;

	ActionObject(const ActionObject&) {}	// Not permitted

	ActionObject& operator = (const ActionObject&) { return *this; }	// Not permitted
};

	}
}

#endif	// traktor_flash_ActionObject_H
