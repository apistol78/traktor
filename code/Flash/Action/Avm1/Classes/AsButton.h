#ifndef traktor_flash_AsButton_H
#define traktor_flash_AsButton_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Button class.
 * \ingroup Flash
 */
class AsButton : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsButton();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Button_get_alpha(CallArgs& ca);

	void Button_set_alpha(CallArgs& ca);

	void Button_get_blendMode(CallArgs& ca);

	void Button_set_blendMode(CallArgs& ca);

	void Button_get_cacheAsBitmap(CallArgs& ca);

	void Button_set_cacheAsBitmap(CallArgs& ca);

	void Button_get_enabled(CallArgs& ca);

	void Button_set_enabled(CallArgs& ca);

	void Button_get_filters(CallArgs& ca);

	void Button_set_filters(CallArgs& ca);

	void Button_get_focusrect(CallArgs& ca);

	void Button_set_focusrect(CallArgs& ca);

	void Button_get_height(CallArgs& ca);

	void Button_set_height(CallArgs& ca);

	void Button_get_highquality(CallArgs& ca);

	void Button_set_highquality(CallArgs& ca);

	void Button_get_menu(CallArgs& ca);

	void Button_set_menu(CallArgs& ca);

	void Button_get_name(CallArgs& ca);

	void Button_set_name(CallArgs& ca);

	void Button_get_parent(CallArgs& ca);

	void Button_set_parent(CallArgs& ca);

	void Button_get_quality(CallArgs& ca);

	void Button_set_quality(CallArgs& ca);

	void Button_get_rotation(CallArgs& ca);

	void Button_set_rotation(CallArgs& ca);

	void Button_get_scale9Grid(CallArgs& ca);

	void Button_set_scale9Grid(CallArgs& ca);

	void Button_get_soundbuftime(CallArgs& ca);

	void Button_set_soundbuftime(CallArgs& ca);

	void Button_get_tabEnabled(CallArgs& ca);

	void Button_set_tabEnabled(CallArgs& ca);

	void Button_get_tabIndex(CallArgs& ca);

	void Button_set_tabIndex(CallArgs& ca);

	void Button_get_target(CallArgs& ca);

	void Button_get_trackAsMenu(CallArgs& ca);

	void Button_set_trackAsMenu(CallArgs& ca);

	void Button_get_url(CallArgs& ca);

	void Button_set_url(CallArgs& ca);

	void Button_get_useHandCursor(CallArgs& ca);

	void Button_set_useHandCursor(CallArgs& ca);

	void Button_get_visible(CallArgs& ca);

	void Button_set_visible(CallArgs& ca);

	void Button_get_width(CallArgs& ca);

	void Button_set_width(CallArgs& ca);

	void Button_get_x(CallArgs& ca);

	void Button_set_x(CallArgs& ca);

	void Button_get_xmouse(CallArgs& ca);

	void Button_get_xscale(CallArgs& ca);

	void Button_set_xscale(CallArgs& ca);

	void Button_get_y(CallArgs& ca);

	void Button_set_y(CallArgs& ca);

	void Button_get_ymouse(CallArgs& ca);

	void Button_get_yscale(CallArgs& ca);

	void Button_set_yscale(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsButton_H
