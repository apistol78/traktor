#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

class Boolean;

/*! \brief Boolean class.
 * \ingroup Spark
 */
class AsBoolean : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsBoolean(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	std::string Boolean_toString(Boolean* self) const;

	bool Boolean_valueOf(Boolean* self) const;
};

	}
}

