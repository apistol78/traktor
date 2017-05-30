/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsButton_H
#define traktor_flash_AsButton_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ButtonInstance;

/*! \brief Button class.
 * \ingroup Flash
 */
class AsButton : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsButton(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void Button_get_alpha(const ButtonInstance* self) const;

	void Button_set_alpha(ButtonInstance* self) const;

	void Button_get_blendMode(const ButtonInstance* self) const;

	void Button_set_blendMode(ButtonInstance* self) const;

	void Button_get_cacheAsBitmap(const ButtonInstance* self) const;

	void Button_set_cacheAsBitmap(ButtonInstance* self) const;

	void Button_get_enabled(const ButtonInstance* self) const;

	void Button_set_enabled(ButtonInstance* self) const;

	void Button_get_filters(const ButtonInstance* self) const;

	void Button_set_filters(ButtonInstance* self) const;

	void Button_get_focusrect(const ButtonInstance* self) const;

	void Button_set_focusrect(ButtonInstance* self) const;

	void Button_get_height(const ButtonInstance* self) const;

	void Button_set_height(ButtonInstance* self) const;

	void Button_get_highquality(const ButtonInstance* self) const;

	void Button_set_highquality(ButtonInstance* self) const;

	void Button_get_menu(const ButtonInstance* self) const;

	void Button_set_menu(ButtonInstance* self) const;

	std::string Button_get_name(const ButtonInstance* self) const;

	void Button_set_name(ButtonInstance* self, const std::string& name) const;

	void Button_get_parent(const ButtonInstance* self) const;

	void Button_set_parent(ButtonInstance* self) const;

	void Button_get_quality(const ButtonInstance* self) const;

	void Button_set_quality(ButtonInstance* self) const;

	void Button_get_rotation(const ButtonInstance* self) const;

	void Button_set_rotation(ButtonInstance* self) const;

	void Button_get_scale9Grid(const ButtonInstance* self) const;

	void Button_set_scale9Grid(ButtonInstance* self) const;

	void Button_get_soundbuftime(const ButtonInstance* self) const;

	void Button_set_soundbuftime(ButtonInstance* self) const;

	void Button_get_tabEnabled(const ButtonInstance* self) const;

	void Button_set_tabEnabled(ButtonInstance* self) const;

	void Button_get_tabIndex(const ButtonInstance* self) const;

	void Button_set_tabIndex(ButtonInstance* self) const;

	void Button_get_target(const ButtonInstance* self) const;

	void Button_get_trackAsMenu(const ButtonInstance* self) const;

	void Button_set_trackAsMenu(ButtonInstance* self) const;

	void Button_get_url(const ButtonInstance* self) const;

	void Button_set_url(ButtonInstance* self) const;

	void Button_get_useHandCursor(const ButtonInstance* self) const;

	void Button_set_useHandCursor(ButtonInstance* self) const;

	void Button_get_visible(const ButtonInstance* self) const;

	void Button_set_visible(ButtonInstance* self) const;

	void Button_get_width(const ButtonInstance* self) const;

	void Button_set_width(ButtonInstance* self) const;

	void Button_get_x(const ButtonInstance* self) const;

	void Button_set_x(ButtonInstance* self) const;

	void Button_get_xmouse(const ButtonInstance* self) const;

	void Button_get_xscale(const ButtonInstance* self) const;

	void Button_set_xscale(ButtonInstance* self) const;

	void Button_get_y(const ButtonInstance* self) const;

	void Button_set_y(ButtonInstance* self) const;

	void Button_get_ymouse(const ButtonInstance* self) const;

	void Button_get_yscale(const ButtonInstance* self) const;

	void Button_set_yscale(ButtonInstance* self) const;
};

	}
}

#endif	// traktor_flash_AsButton_H
