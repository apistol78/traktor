/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_display_BitmapData_H
#define traktor_flash_As_flash_display_BitmapData_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_flash_display_BitmapData : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_display_BitmapData(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;
};

	}
}

#endif	// traktor_flash_As_flash_display_BitmapData_H
