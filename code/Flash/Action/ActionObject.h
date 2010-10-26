#ifndef traktor_flash_ActionObject_H
#define traktor_flash_ActionObject_H

#include <map>
#include <string>
#include "Core/Object.h"
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

/*! \brief ActionScript object.
 * \ingroup Flash
 */
class T_DLLCLASS ActionObject : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::map< std::string, ActionValue > member_map_t;
	typedef std::map< std::string, std::pair< Ref< ActionFunction >, Ref< ActionFunction > > > property_map_t;

	ActionObject();

	explicit ActionObject(const std::string& prototypeName);

	explicit ActionObject(ActionObject* prototype);

	virtual ~ActionObject();

	virtual void addRef(void* owner) const
	{
		m_traceColor = TcBlack;
		Object::addRef(owner);
	}
	
	virtual void release(void* owner) const;

	virtual void addInterface(ActionObject* intrface);

	virtual ActionObject* getPrototype(ActionContext* context);

	virtual void setMember(const std::string& memberName, const ActionValue& memberValue);

	virtual bool getMember(ActionContext* context, const std::string& memberName, ActionValue& outMemberValue);

	virtual bool deleteMember(const std::string& memberName);

	virtual void deleteAllMembers();

	virtual void addProperty(const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet);

	virtual bool getPropertyGet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertyGet);

	virtual bool getPropertySet(ActionContext* context, const std::string& propertyName, Ref< ActionFunction >& outPropertySet);

	virtual const property_map_t& getProperties() const;

	virtual void deleteAllProperties();

	virtual avm_number_t valueOf() const;

	virtual ActionValue toString() const;

	void setReadOnly();

	bool getLocalMember(const std::string& memberName, ActionValue& outMemberValue) const;

	bool hasOwnProperty(const std::string& propertyName) const;

	bool getLocalPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet) const;

	bool getLocalPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet) const;

protected:
	struct IVisitor
	{
		virtual void operator () (ActionObject* memberObject) const = 0;
	};

	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	friend class ActionObjectCyclic;

	enum TraceColor
	{
		TcBlack,
		TcPurple,
		TcGray,
		TcWhite
	};

	struct MarkGrayVisitor : public IVisitor
	{
		virtual void operator () (ActionObject* memberObject) const;
	};

	struct ScanVisitor : public IVisitor
	{
		virtual void operator () (ActionObject* memberObject) const;
	};

	struct ScanBlackVisitor : public IVisitor
	{
		virtual void operator () (ActionObject* memberObject) const;
	};

	bool m_readOnly;
	mutable member_map_t m_members;
	property_map_t m_properties;
	mutable int32_t m_traceColor;
	mutable bool m_traceBuffered;
	mutable int32_t m_traceRefCount;

	void traceMarkGray();

	void traceScan();

	void traceScanBlack();

	void traceCollectWhite();
};

	}
}

#endif	// traktor_flash_ActionObject_H
