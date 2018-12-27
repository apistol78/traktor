/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsBoolean_H
#define traktor_flash_AsBoolean_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

class Boolean;

/*! \brief Boolean class.
 * \ingroup Flash
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

#endif	// traktor_flash_AsBoolean_H
