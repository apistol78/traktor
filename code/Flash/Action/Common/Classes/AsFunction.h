/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsFunction_H
#define traktor_flash_AsFunction_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Function class.
 * \ingroup Flash
 */
class AsFunction : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsFunction(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Function_apply(CallArgs& ca);

	void Function_call(CallArgs& ca);

	void Function_toString(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsFunction_H
