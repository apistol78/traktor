/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_geom_Transform_H
#define traktor_flash_As_flash_geom_Transform_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ColorTransform;
class Transform;

class As_flash_geom_Transform : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Transform(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	Ref< ColorTransform > Transform_get_colorTransform(Transform* self) const;

	void Transform_set_colorTransform(Transform* self, const ColorTransform* colorTransform) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Transform_H
