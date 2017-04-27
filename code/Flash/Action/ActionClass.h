/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionClass_H
#define traktor_flash_ActionClass_H

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

class ActionContext;

/*! \brief Base class for all native ActionScript classes.
 * \ingroup Flash
 */
class T_DLLCLASS ActionClass : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionClass(ActionContext* context, const char* className);

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) T_OVERRIDE;

	/*! \brief Initialize instance of class.
	 *
	 * Called before constructor in order to
	 * prepare instance members, properties etc.
	 */
	virtual void initialize(ActionObject* self) = 0;

	/*! \brief Construct instance.
	 *
	 * Called after instance has been initialized and
	 * constructor has been invoked through "new" operator.
	 */
	virtual void construct(ActionObject* self, const ActionValueArray& args) = 0;

	/*! \brief Constructor called.
	 *
	 * Constructor explicitly called as a
	 * function.
	 */
	virtual ActionValue xplicit(const ActionValueArray& args) = 0;
};

	}
}

#endif	// traktor_flash_ActionClass_H
