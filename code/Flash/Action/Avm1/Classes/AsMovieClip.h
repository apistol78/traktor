#ifndef traktor_flash_AsMovieClip_H
#define traktor_flash_AsMovieClip_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief MovieClip class.
 * \ingroup Flash
 */
class AsMovieClip : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsMovieClip > getInstance();

private:
	AsMovieClip();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void MovieClip_attachAudio(CallArgs& ca);

	void MovieClip_attachBitmap(CallArgs& ca);

	void MovieClip_attachMovie(CallArgs& ca);

	void MovieClip_beginBitmapFill(CallArgs& ca);

	void MovieClip_beginFill(CallArgs& ca);

	void MovieClip_beginGradientFill(CallArgs& ca);

	void MovieClip_clear(CallArgs& ca);

	void MovieClip_createEmptyMovieClip(CallArgs& ca);

	void MovieClip_createTextField(CallArgs& ca);

	void MovieClip_curveTo(CallArgs& ca);

	void MovieClip_duplicateMovieClip(CallArgs& ca);

	void MovieClip_endFill(CallArgs& ca);

	void MovieClip_getBounds(CallArgs& ca);

	void MovieClip_getBytesLoaded(CallArgs& ca);

	void MovieClip_getBytesTotal(CallArgs& ca);

	void MovieClip_getDepth(CallArgs& ca);

	void MovieClip_getInstanceAtDepth(CallArgs& ca);

	void MovieClip_getNextHighestDepth(CallArgs& ca);

	void MovieClip_getRect(CallArgs& ca);

	void MovieClip_getSWFVersion(CallArgs& ca);

	void MovieClip_getTextSnapshot(CallArgs& ca);

	void MovieClip_getURL(CallArgs& ca);

	void MovieClip_globalToLocal(CallArgs& ca);

	void MovieClip_gotoAndPlay(CallArgs& ca);

	void MovieClip_gotoAndStop(CallArgs& ca);

	void MovieClip_hitTest(CallArgs& ca);

	void MovieClip_lineGradientStyle(CallArgs& ca);

	void MovieClip_lineStyle(CallArgs& ca);

	void MovieClip_lineTo(CallArgs& ca);

	void MovieClip_loadMovie(CallArgs& ca);

	void MovieClip_loadVariables(CallArgs& ca);

	void MovieClip_localToGlobal(CallArgs& ca);

	void MovieClip_moveTo(CallArgs& ca);

	void MovieClip_nextFrame(CallArgs& ca);

	void MovieClip_play(CallArgs& ca);

	void MovieClip_prevFrame(CallArgs& ca);

	void MovieClip_removeMovieClip(CallArgs& ca);

	void MovieClip_setMask(CallArgs& ca);

	void MovieClip_startDrag(CallArgs& ca);

	void MovieClip_stop(CallArgs& ca);

	void MovieClip_stopDrag(CallArgs& ca);

	void MovieClip_swapDepths(CallArgs& ca);

	void MovieClip_unloadMovie(CallArgs& ca);

	void MovieClip_get_alpha(CallArgs& ca);

	void MovieClip_set_alpha(CallArgs& ca);

	void MovieClip_get_blendMode(CallArgs& ca);

	void MovieClip_set_blendMode(CallArgs& ca);

	void MovieClip_get_cacheAsBitmap(CallArgs& ca);

	void MovieClip_set_cacheAsBitmap(CallArgs& ca);

	void MovieClip_get_currentframe(CallArgs& ca);

	void MovieClip_set_currentframe(CallArgs& ca);

	void MovieClip_get_droptarget(CallArgs& ca);

	void MovieClip_set_droptarget(CallArgs& ca);

	void MovieClip_get_enabled(CallArgs& ca);

	void MovieClip_set_enabled(CallArgs& ca);

	void MovieClip_get_filters(CallArgs& ca);

	void MovieClip_set_filters(CallArgs& ca);

	void MovieClip_get_focusEnabled(CallArgs& ca);

	void MovieClip_set_focusEnabled(CallArgs& ca);

	void MovieClip_get_focusrect(CallArgs& ca);

	void MovieClip_set_focusrect(CallArgs& ca);

	void MovieClip_get_forceSmoothing(CallArgs& ca);

	void MovieClip_set_forceSmoothing(CallArgs& ca);

	void MovieClip_get_framesloaded(CallArgs& ca);

	void MovieClip_set_framesloaded(CallArgs& ca);

	void MovieClip_get_height(CallArgs& ca);

	void MovieClip_set_height(CallArgs& ca);

	void MovieClip_get_highquality(CallArgs& ca);

	void MovieClip_set_highquality(CallArgs& ca);

	void MovieClip_get_hitArea(CallArgs& ca);

	void MovieClip_set_hitArea(CallArgs& ca);

	void MovieClip_get_lockroot(CallArgs& ca);

	void MovieClip_set_lockroot(CallArgs& ca);

	void MovieClip_get_menu(CallArgs& ca);

	void MovieClip_set_menu(CallArgs& ca);

	void MovieClip_get_name(CallArgs& ca);

	void MovieClip_set_name(CallArgs& ca);

	void MovieClip_get_opaqueBackground(CallArgs& ca);

	void MovieClip_set_opaqueBackground(CallArgs& ca);

	void MovieClip_get_parent(CallArgs& ca);

	void MovieClip_set_parent(CallArgs& ca);

	void MovieClip_get_quality(CallArgs& ca);

	void MovieClip_set_quality(CallArgs& ca);

	void MovieClip_get_rotation(CallArgs& ca);

	void MovieClip_set_rotation(CallArgs& ca);

	void MovieClip_get_scale9Grid(CallArgs& ca);

	void MovieClip_set_scale9Grid(CallArgs& ca);

	void MovieClip_get_scrollRect(CallArgs& ca);

	void MovieClip_set_scrollRect(CallArgs& ca);

	void MovieClip_get_soundbuftime(CallArgs& ca);

	void MovieClip_set_soundbuftime(CallArgs& ca);

	void MovieClip_get_tabChildren(CallArgs& ca);

	void MovieClip_set_tabChildren(CallArgs& ca);

	void MovieClip_get_tabEnabled(CallArgs& ca);

	void MovieClip_set_tabEnabled(CallArgs& ca);

	void MovieClip_get_tabIndex(CallArgs& ca);

	void MovieClip_set_tabIndex(CallArgs& ca);

	void MovieClip_get_target(CallArgs& ca);

	void MovieClip_set_target(CallArgs& ca);

	void MovieClip_get_totalframes(CallArgs& ca);

	void MovieClip_set_totalframes(CallArgs& ca);

	void MovieClip_get_trackAsMenu(CallArgs& ca);

	void MovieClip_set_trackAsMenu(CallArgs& ca);

	void MovieClip_get_transform(CallArgs& ca);

	void MovieClip_set_transform(CallArgs& ca);

	void MovieClip_get_url(CallArgs& ca);

	void MovieClip_set_url(CallArgs& ca);

	void MovieClip_get_useHandCursor(CallArgs& ca);

	void MovieClip_set_useHandCursor(CallArgs& ca);

	void MovieClip_get_visible(CallArgs& ca);

	void MovieClip_set_visible(CallArgs& ca);

	void MovieClip_get_width(CallArgs& ca);

	void MovieClip_set_width(CallArgs& ca);

	void MovieClip_get_x(CallArgs& ca);

	void MovieClip_set_x(CallArgs& ca);

	void MovieClip_get_xmouse(CallArgs& ca);

	void MovieClip_set_xmouse(CallArgs& ca);

	void MovieClip_get_xscale(CallArgs& ca);

	void MovieClip_set_xscale(CallArgs& ca);

	void MovieClip_get_y(CallArgs& ca);

	void MovieClip_set_y(CallArgs& ca);

	void MovieClip_get_ymouse(CallArgs& ca);

	void MovieClip_set_ymouse(CallArgs& ca);

	void MovieClip_get_yscale(CallArgs& ca);

	void MovieClip_set_yscale(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMovieClip_H
