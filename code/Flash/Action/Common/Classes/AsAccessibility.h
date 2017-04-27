/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsAccessibility_H
#define traktor_flash_AsAccessibility_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Accessibility class.
 * \ingroup Flash
 */
class AsAccessibility : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsAccessibility(ActionContext* context);

private:
	void Accessibility_isActive(CallArgs& ca);

	void Accessibility_updateProperties(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsAccessibility_H
