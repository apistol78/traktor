#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

class Number;

/*! \brief Number class.
 * \ingroup Spark
 */
class AsNumber : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsNumber(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	std::wstring Number_toString(const Number* self) const;

	float Number_valueOf(const Number* self) const;
};

	}
}

