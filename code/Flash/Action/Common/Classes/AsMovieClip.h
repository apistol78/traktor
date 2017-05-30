/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsMovieClip_H
#define traktor_flash_AsMovieClip_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class Array;
class BitmapData;
class CharacterInstance;
class EditInstance;
class SpriteInstance;
class Matrix;
class Transform;

/*! \brief MovieClip class.
 * \ingroup Flash
 */
class AsMovieClip : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsMovieClip(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void MovieClip_attachAudio(SpriteInstance* self) const;

	void MovieClip_attachBitmap_2(SpriteInstance* self, BitmapData* bmp, int32_t depth) const;

	void MovieClip_attachBitmap_3(SpriteInstance* self, BitmapData* bmp, int32_t depth, const std::string& pixelSnapping) const;

	void MovieClip_attachBitmap_4(SpriteInstance* self, BitmapData* bmp, int32_t depth, const std::string& pixelSnapping, bool smoothing) const;

	Ref< SpriteInstance > MovieClip_attachMovie_3(SpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth) const;

	Ref< SpriteInstance > MovieClip_attachMovie_4(SpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth, ActionObject* initObject) const;

	void MovieClip_beginBitmapFill(SpriteInstance* self, BitmapData* bm, const Matrix* matrix, bool repeat, bool smoothing) const;

	void MovieClip_beginFill_1(SpriteInstance* self, uint32_t rgb) const;

	void MovieClip_beginFill_2(SpriteInstance* self, uint32_t rgb, int32_t alpha) const;

	bool MovieClip_beginGradientFill(SpriteInstance* self, const std::string& fillType, const Array* colors, const Array* alphas, const Array* ratios, ActionObject* matrix) const;

	void MovieClip_clear(SpriteInstance* self) const;

	Ref< SpriteInstance > MovieClip_createEmptyMovieClip(SpriteInstance* self, const std::string& emptyClipName, int32_t depth) const;

	Ref< EditInstance > MovieClip_createTextField(
		SpriteInstance* self,
		const std::string& name,
		int32_t depth,
		float x,
		float y,
		float width,
		float height
	) const;

	void MovieClip_curveTo(SpriteInstance* self, float controlX, float controlY, float anchorX, float anchorY) const;

	Ref< SpriteInstance > MovieClip_duplicateMovieClip(SpriteInstance* self, const std::string& name, int32_t depth) const;

	void MovieClip_endFill(SpriteInstance* self) const;

	Ref< ActionObject > MovieClip_getBounds(SpriteInstance* self, const CharacterInstance* reference) const;

	int32_t MovieClip_getBytesLoaded(SpriteInstance* self) const;

	int32_t MovieClip_getBytesTotal(SpriteInstance* self) const;

	int32_t MovieClip_getDepth(SpriteInstance* self) const;

	Ref< CharacterInstance > MovieClip_getInstanceAtDepth(SpriteInstance* self, int32_t depth) const;

	int32_t MovieClip_getNextHighestDepth(SpriteInstance* self) const;

	Ref< ActionObject > MovieClip_getRect(SpriteInstance* self, const CharacterInstance* reference) const;

	int32_t MovieClip_getSWFVersion(SpriteInstance* self) const;

	void MovieClip_getTextSnapshot(SpriteInstance* self) const;

	void MovieClip_getURL(SpriteInstance* self, const std::wstring& url) const;

	void MovieClip_globalToLocal(SpriteInstance* self) const;

	void MovieClip_gotoAndPlay(SpriteInstance* self, const ActionValue& arg0) const;

	void MovieClip_gotoAndStop(SpriteInstance* self, const ActionValue& arg0) const;

	bool MovieClip_hitTest_1(const SpriteInstance* self, const CharacterInstance* shape) const;

	bool MovieClip_hitTest_2(const SpriteInstance* self, float x, float y) const;

	void MovieClip_lineGradientStyle(SpriteInstance* self) const;

	void MovieClip_lineStyle(SpriteInstance* self) const;

	void MovieClip_lineTo(SpriteInstance* self, float x, float y) const;

	Ref< SpriteInstance > MovieClip_loadMovie(SpriteInstance* self, const std::wstring& fileName) const;

	void MovieClip_loadVariables(SpriteInstance* self) const;

	void MovieClip_localToGlobal(const SpriteInstance* self) const;

	void MovieClip_moveTo(SpriteInstance* self, float x, float y) const;

	void MovieClip_nextFrame(SpriteInstance* self) const;

	void MovieClip_play(SpriteInstance* self) const;

	void MovieClip_prevFrame(SpriteInstance* self) const;

	void MovieClip_removeMovieClip(SpriteInstance* self) const;

	void MovieClip_setMask(SpriteInstance* self, SpriteInstance* mask) const;

	void MovieClip_startDrag(SpriteInstance* self) const;

	void MovieClip_stop(SpriteInstance* self) const;

	void MovieClip_stopDrag(SpriteInstance* self) const;

	void MovieClip_swapDepths(SpriteInstance* self, const ActionValue& arg0) const;

	std::string MovieClip_toString(const SpriteInstance* self) const;

	void MovieClip_unloadMovie(SpriteInstance* self) const;

	float MovieClip_get_alpha(const SpriteInstance* self) const;

	void MovieClip_set_alpha(SpriteInstance* self, float alpha) const;

	void MovieClip_get_blendMode(const SpriteInstance* self) const;

	void MovieClip_set_blendMode(SpriteInstance* self) const;

	bool MovieClip_get_cacheAsBitmap(const SpriteInstance* self) const;

	void MovieClip_set_cacheAsBitmap(SpriteInstance* self, bool enabled) const;

	int32_t MovieClip_get_currentframe(const SpriteInstance* self) const;

	void MovieClip_get_droptarget(const SpriteInstance* self) const;

	bool MovieClip_get_enabled(const SpriteInstance* self) const;

	void MovieClip_set_enabled(SpriteInstance* self, bool enabled) const;

	void MovieClip_get_filters(const SpriteInstance* self) const;

	void MovieClip_set_filters(SpriteInstance* self) const;

	void MovieClip_get_focusEnabled(const SpriteInstance* self) const;

	void MovieClip_set_focusEnabled(SpriteInstance* self) const;

	void MovieClip_get_focusrect(const SpriteInstance* self) const;

	void MovieClip_set_focusrect(SpriteInstance* self) const;

	void MovieClip_get_forceSmoothing(const SpriteInstance* self) const;

	void MovieClip_set_forceSmoothing(SpriteInstance* self) const;

	int32_t MovieClip_get_framesloaded(const SpriteInstance* self) const;

	float MovieClip_get_height(const SpriteInstance* self) const;

	void MovieClip_set_height(SpriteInstance* self, float height) const;

	void MovieClip_get_highquality(const SpriteInstance* self) const;

	void MovieClip_set_highquality(SpriteInstance* self) const;

	void MovieClip_get_hitArea(const SpriteInstance* self) const;

	void MovieClip_set_hitArea(SpriteInstance* self) const;

	void MovieClip_get_lockroot(const SpriteInstance* self) const;

	void MovieClip_set_lockroot(SpriteInstance* self) const;

	void MovieClip_get_menu(const SpriteInstance* self) const;

	void MovieClip_set_menu(SpriteInstance* self) const;

	ActionValue MovieClip_get_name(const SpriteInstance* self) const;

	void MovieClip_set_name(SpriteInstance* self, const std::string& name) const;

	void MovieClip_get_opaqueBackground(const SpriteInstance* self) const;

	void MovieClip_set_opaqueBackground(SpriteInstance* self, const ActionValue& value) const;

	CharacterInstance* MovieClip_get_parent(SpriteInstance* self) const;

	void MovieClip_set_parent(SpriteInstance* self) const;

	std::string MovieClip_get_quality(const SpriteInstance* self) const;

	void MovieClip_set_quality(SpriteInstance* self, const std::string& quality) const;

	float MovieClip_get_rotation(const SpriteInstance* self) const;

	void MovieClip_set_rotation(SpriteInstance* self, float rotation) const;

	void MovieClip_get_scale9Grid(const SpriteInstance* self) const;

	void MovieClip_set_scale9Grid(SpriteInstance* self) const;

	void MovieClip_get_scrollRect(const SpriteInstance* self) const;

	void MovieClip_set_scrollRect(SpriteInstance* self) const;

	void MovieClip_get_soundbuftime(const SpriteInstance* self) const;

	void MovieClip_set_soundbuftime(SpriteInstance* self) const;

	void MovieClip_get_tabChildren(const SpriteInstance* self) const;

	void MovieClip_set_tabChildren(SpriteInstance* self) const;

	void MovieClip_get_tabEnabled(const SpriteInstance* self) const;

	void MovieClip_set_tabEnabled(SpriteInstance* self) const;

	void MovieClip_get_tabIndex(const SpriteInstance* self) const;

	void MovieClip_set_tabIndex(SpriteInstance* self) const;

	std::string MovieClip_get_target(const SpriteInstance* self) const;

	uint32_t MovieClip_get_totalframes(const SpriteInstance* self) const;

	void MovieClip_get_trackAsMenu(const SpriteInstance* self) const;

	void MovieClip_set_trackAsMenu(SpriteInstance* self) const;

	Ref< Transform > MovieClip_get_transform(SpriteInstance* self) const;

	std::string MovieClip_get_url(const SpriteInstance* self) const;

	void MovieClip_get_useHandCursor(const SpriteInstance* self) const;

	void MovieClip_set_useHandCursor(SpriteInstance* self) const;

	bool MovieClip_get_visible(const SpriteInstance* self) const;

	void MovieClip_set_visible(SpriteInstance* self, bool visible) const;

	float MovieClip_get_width(const SpriteInstance* self) const;

	void MovieClip_set_width(SpriteInstance* self, float width) const;

	float MovieClip_get_x(const SpriteInstance* self) const;

	void MovieClip_set_x(SpriteInstance* self, float x) const;

	int32_t MovieClip_get_xmouse(const SpriteInstance* self) const;

	float MovieClip_get_xscale(const SpriteInstance* self) const;

	void MovieClip_set_xscale(SpriteInstance* self, float x) const;

	float MovieClip_get_y(const SpriteInstance* self) const;

	void MovieClip_set_y(SpriteInstance* self, float y) const;

	int32_t MovieClip_get_ymouse(const SpriteInstance* self) const;

	float MovieClip_get_yscale(const SpriteInstance* self) const;

	void MovieClip_set_yscale(SpriteInstance* self, float y) const;
};

	}
}

#endif	// traktor_flash_AsMovieClip_H
