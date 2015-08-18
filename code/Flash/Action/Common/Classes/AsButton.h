#ifndef traktor_flash_AsButton_H
#define traktor_flash_AsButton_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class FlashButtonInstance;

/*! \brief Button class.
 * \ingroup Flash
 */
class AsButton : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsButton(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	void Button_get_alpha(const FlashButtonInstance* self) const;

	void Button_set_alpha(FlashButtonInstance* self) const;

	void Button_get_blendMode(const FlashButtonInstance* self) const;

	void Button_set_blendMode(FlashButtonInstance* self) const;

	void Button_get_cacheAsBitmap(const FlashButtonInstance* self) const;

	void Button_set_cacheAsBitmap(FlashButtonInstance* self) const;

	void Button_get_enabled(const FlashButtonInstance* self) const;

	void Button_set_enabled(FlashButtonInstance* self) const;

	void Button_get_filters(const FlashButtonInstance* self) const;

	void Button_set_filters(FlashButtonInstance* self) const;

	void Button_get_focusrect(const FlashButtonInstance* self) const;

	void Button_set_focusrect(FlashButtonInstance* self) const;

	void Button_get_height(const FlashButtonInstance* self) const;

	void Button_set_height(FlashButtonInstance* self) const;

	void Button_get_highquality(const FlashButtonInstance* self) const;

	void Button_set_highquality(FlashButtonInstance* self) const;

	void Button_get_menu(const FlashButtonInstance* self) const;

	void Button_set_menu(FlashButtonInstance* self) const;

	std::string Button_get_name(const FlashButtonInstance* self) const;

	void Button_set_name(FlashButtonInstance* self, const std::string& name) const;

	void Button_get_parent(const FlashButtonInstance* self) const;

	void Button_set_parent(FlashButtonInstance* self) const;

	void Button_get_quality(const FlashButtonInstance* self) const;

	void Button_set_quality(FlashButtonInstance* self) const;

	void Button_get_rotation(const FlashButtonInstance* self) const;

	void Button_set_rotation(FlashButtonInstance* self) const;

	void Button_get_scale9Grid(const FlashButtonInstance* self) const;

	void Button_set_scale9Grid(FlashButtonInstance* self) const;

	void Button_get_soundbuftime(const FlashButtonInstance* self) const;

	void Button_set_soundbuftime(FlashButtonInstance* self) const;

	void Button_get_tabEnabled(const FlashButtonInstance* self) const;

	void Button_set_tabEnabled(FlashButtonInstance* self) const;

	void Button_get_tabIndex(const FlashButtonInstance* self) const;

	void Button_set_tabIndex(FlashButtonInstance* self) const;

	void Button_get_target(const FlashButtonInstance* self) const;

	void Button_get_trackAsMenu(const FlashButtonInstance* self) const;

	void Button_set_trackAsMenu(FlashButtonInstance* self) const;

	void Button_get_url(const FlashButtonInstance* self) const;

	void Button_set_url(FlashButtonInstance* self) const;

	void Button_get_useHandCursor(const FlashButtonInstance* self) const;

	void Button_set_useHandCursor(FlashButtonInstance* self) const;

	void Button_get_visible(const FlashButtonInstance* self) const;

	void Button_set_visible(FlashButtonInstance* self) const;

	void Button_get_width(const FlashButtonInstance* self) const;

	void Button_set_width(FlashButtonInstance* self) const;

	void Button_get_x(const FlashButtonInstance* self) const;

	void Button_set_x(FlashButtonInstance* self) const;

	void Button_get_xmouse(const FlashButtonInstance* self) const;

	void Button_get_xscale(const FlashButtonInstance* self) const;

	void Button_set_xscale(FlashButtonInstance* self) const;

	void Button_get_y(const FlashButtonInstance* self) const;

	void Button_set_y(FlashButtonInstance* self) const;

	void Button_get_ymouse(const FlashButtonInstance* self) const;

	void Button_get_yscale(const FlashButtonInstance* self) const;

	void Button_set_yscale(FlashButtonInstance* self) const;
};

	}
}

#endif	// traktor_flash_AsButton_H
