/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsMath_H
#define traktor_flash_AsMath_H

#include "Core/Math/Random.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Math class.
 * \ingroup Flash
 */
class AsMath : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsMath(ActionContext* context);

private:
	Random m_random;

	void Math_abs(CallArgs& ca);

	void Math_acos(CallArgs& ca);

	void Math_asin(CallArgs& ca);

	void Math_atan(CallArgs& ca);

	void Math_atan2(CallArgs& ca);

	void Math_ceil(CallArgs& ca);

	void Math_cos(CallArgs& ca);

	void Math_exp(CallArgs& ca);

	void Math_floor(CallArgs& ca);

	void Math_log(CallArgs& ca);

	void Math_max(CallArgs& ca);

	void Math_min(CallArgs& ca);

	void Math_pow(CallArgs& ca);

	void Math_random(CallArgs& ca);

	void Math_round(CallArgs& ca);

	void Math_sin(CallArgs& ca);

	void Math_sqrt(CallArgs& ca);

	void Math_tan(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMath_H
