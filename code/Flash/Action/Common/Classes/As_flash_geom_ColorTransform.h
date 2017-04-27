/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_geom_ColorTransform_H
#define traktor_flash_As_flash_geom_ColorTransform_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ColorTransform;

class As_flash_geom_ColorTransform : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_ColorTransform(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	float ColorTransform_get_alphaMultiplier(ColorTransform* self) const;

	void ColorTransform_set_alphaMultiplier(ColorTransform* self, float value) const;

	float ColorTransform_get_alphaOffset(ColorTransform* self) const;

	void ColorTransform_set_alphaOffset(ColorTransform* self, float value) const;

	float ColorTransform_get_blueMultiplier(ColorTransform* self) const;

	void ColorTransform_set_blueMultiplier(ColorTransform* self, float value) const;

	float ColorTransform_get_blueOffset(ColorTransform* self) const;

	void ColorTransform_set_blueOffset(ColorTransform* self, float value) const;

	float ColorTransform_get_greenMultiplier(ColorTransform* self) const;

	void ColorTransform_set_greenMultiplier(ColorTransform* self, float value) const;

	float ColorTransform_get_greenOffset(ColorTransform* self) const;

	void ColorTransform_set_greenOffset(ColorTransform* self, float value) const;

	float ColorTransform_get_redMultiplier(ColorTransform* self) const;

	void ColorTransform_set_redMultiplier(ColorTransform* self, float value) const;

	float ColorTransform_get_redOffset(ColorTransform* self) const;

	void ColorTransform_set_redOffset(ColorTransform* self, float value) const;

	float ColorTransform_get_rgb(ColorTransform* self) const;

	void ColorTransform_set_rgb(ColorTransform* self, float value) const;

	std::wstring ColorTransform_toString(const ColorTransform* self) const;
};

	}
}

#endif	// traktor_flash_As_flash_geom_ColorTransform_H
