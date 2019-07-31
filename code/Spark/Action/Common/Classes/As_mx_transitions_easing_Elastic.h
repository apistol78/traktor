#pragma once

#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

class As_mx_transitions_easing_Elastic : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Elastic(ActionContext* context);

private:
	void Elastic_easeIn(CallArgs& ca);

	void Elastic_easeInOut(CallArgs& ca);

	void Elastic_easeOut(CallArgs& ca);
};

	}
}

