#ifndef traktor_flash_ActionObject_H
#define traktor_flash_ActionObject_H

//#include <map>
#include <string>
#include "Core/Containers/SmallMap.h"
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
class ActionObjectRelay;

/*! \brief ActionScript object.
 * \ingroup Flash
 */
class T_DLLCLASS ActionObject : public Collectable
{
	T_RTTI_CLASS;

public:
	typedef SmallMap< uint32_t, ActionValue > member_map_t;
	typedef SmallMap< uint32_t, std::pair< Ref< ActionFunction >, Ref< ActionFunction > > > property_map_t;

	explicit ActionObject(ActionContext* context, ActionObjectRelay* relay = 0);

	explicit ActionObject(ActionContext* context, const std::string& prototypeName, ActionObjectRelay* relay = 0);

	explicit ActionObject(ActionContext* context, ActionObject* prototype, ActionObjectRelay* relay = 0);

	virtual void release(void* owner) const;

	virtual void addInterface(ActionObject* intrface);

	void set__proto__(ActionObject* prototype);

	virtual ActionObject* get__proto__();

	virtual void setMember(uint32_t memberName, const ActionValue& memberValue);

	virtual bool getMember(uint32_t memberName, ActionValue& outMemberValue);

	virtual bool deleteMember(uint32_t memberName);

	virtual void deleteAllMembers();

	virtual void addProperty(uint32_t propertyName, ActionFunction* propertyGet, ActionFunction* propertySet);

	virtual bool getPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet);

	virtual bool getPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet);

	virtual const property_map_t& getProperties() const;

	virtual void deleteAllProperties();

	virtual ActionValue valueOf();

	virtual ActionValue toString();

	virtual Ref< ActionObject > getSuper();

	void setReadOnly();

	bool hasOwnMember(uint32_t memberName) const;

	bool getLocalMember(uint32_t memberName, ActionValue& outMemberValue) const;

	bool hasOwnProperty(uint32_t propertyName) const;

	bool getLocalPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet) const;

	bool getLocalPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet) const;

	void setRelay(ActionObjectRelay* relay);

	ActionObjectRelay* getRelay() const { return m_relay; }

	template < typename RelayedType >
	RelayedType* getRelay() const { return dynamic_type_cast< RelayedType* >(getRelay()); }

	ActionContext* getContext() const { return m_context; }

	const member_map_t& getLocalMembers() const { return m_members; }

	/*! \name Helpers
	 * \note Do not use in time critical paths.
	 */
	// \{

	void setMember(const std::string& memberName, const ActionValue& memberValue);

	bool getMember(const std::string& memberName, ActionValue& outMemberValue);

	bool getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const;

	bool getMemberByQName(const std::string& memberName, ActionValue& outMemberValue);

	void addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet);

	bool getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet);

	bool getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet);

	// \}

protected:
	friend class ActionObjectRelay;

	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

	void setOverrideRelay(ActionObjectRelay* relay);

private:
	ActionContext* m_context;
	bool m_readOnly;
	mutable member_map_t m_members;
	property_map_t m_properties;
	Ref< ActionObject > m__proto__;		//!< Cached "__proto__" member value.
	mutable Ref< ActionObjectRelay > m_relay;

	ActionObject(const ActionObject&) {}	// Not permitted

	ActionObject& operator = (const ActionObject&) { return *this; }	// Not permitted
};

	}
}

#endif	// traktor_flash_ActionObject_H
