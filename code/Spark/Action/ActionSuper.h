#pragma once

#include "Spark/Action/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Class wrapper to access parent class members.
 * \ingroup Spark
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

	virtual ActionObject* get__proto__() override;

	virtual void setMember(uint32_t memberName, const ActionValue& memberValue) override;

	virtual bool getMember(uint32_t memberName, ActionValue& outMemberValue) override;

	virtual bool getPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet) override;

	virtual bool getPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet) override;

	virtual ActionValue valueOf() override;

	virtual ActionValue toString() override;

	virtual Ref< ActionObject > getSuper() override;

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) override;

	ActionObject* getObject() { return m_object; }

protected:
	virtual void trace(visitor_t visitor) const override;

	virtual void dereference() override;

private:
	Ref< ActionObject > m_object;
	Ref< ActionFunction > m_superClass;
	Ref< ActionObject > m_superPrototype;
};

	}
}

