#pragma once

#include "Spark/Action/ActionClass.h"

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

struct CallArgs;

/*! Object class.
 * \ingroup Spark
 */
class T_DLLCLASS AsObject : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsObject(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Object_addProperty(ActionObject* self, const std::string& propertyName, ActionFunction* propertyGet, ActionFunction* propertySet) const;

	bool Object_hasOwnProperty(const ActionObject* self, const std::string& propertyName) const;

	bool Object_isPropertyEnumerable(const ActionObject* self) const;

	bool Object_isPrototypeOf(const ActionObject* self) const;

	void Object_registerClass(CallArgs& ca);

	void Object_toString(CallArgs& ca);

	void Object_unwatch(ActionObject* self) const;

	void Object_valueOf(CallArgs& ca);

	void Object_watch(ActionObject* self) const;
};

	}
}

