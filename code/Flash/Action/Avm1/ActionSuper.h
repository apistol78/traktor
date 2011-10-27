#ifndef traktor_flash_ActionSuper_H
#define traktor_flash_ActionSuper_H

#include "Flash/Action/ActionFunction.h"

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

/*! \brief Class wrapper to access parent class members.
 * \ingroup Flash
 *
 * \note Super is accessed through
 * "(object).__proto__.__constructor__" and
 * "(object).__proto__.__proto__"
 * which are relationships that AopExtends prepares.
 */
class T_DLLCLASS ActionSuper : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionSuper(ActionContext* context, ActionObject* object, ActionObject* superPrototype, ActionFunction* superClass);

	virtual ActionObject* get__proto__();

	virtual void setMember(const std::string& memberName, const ActionValue& memberValue);

	virtual bool getMember(const std::string& memberName, ActionValue& outMemberValue);

	virtual bool getPropertyGet(const std::string& propertyName, Ref< ActionFunction >& outPropertyGet);

	virtual bool getPropertySet(const std::string& propertyName, Ref< ActionFunction >& outPropertySet);

	virtual ActionValue valueOf();

	virtual ActionValue toString();

	virtual Ref< ActionObject > getSuper();

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args);

	ActionObject* getObject() { return m_object; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	Ref< ActionObject > m_object;
	Ref< ActionFunction > m_superClass;
	Ref< ActionObject > m_superPrototype;
};

	}
}

#endif	// traktor_flash_ActionSuper_H
