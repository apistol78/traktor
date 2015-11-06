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
class FlashCharacterInstance;
class FlashEditInstance;
class FlashSpriteInstance;
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
	void MovieClip_attachAudio(FlashSpriteInstance* self) const;

	void MovieClip_attachBitmap_2(FlashSpriteInstance* self, const BitmapData* bmp, int32_t depth) const;

	void MovieClip_attachBitmap_3(FlashSpriteInstance* self, const BitmapData* bmp, int32_t depth, const std::string& pixelSnapping) const;

	void MovieClip_attachBitmap_4(FlashSpriteInstance* self, const BitmapData* bmp, int32_t depth, const std::string& pixelSnapping, bool smoothing) const;

	Ref< FlashSpriteInstance > MovieClip_attachMovie_3(FlashSpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth) const;

	Ref< FlashSpriteInstance > MovieClip_attachMovie_4(FlashSpriteInstance* self, const std::string& attachClipName, const std::string& attachClipNewName, int32_t depth, ActionObject* initObject) const;

	void MovieClip_beginBitmapFill(FlashSpriteInstance* self) const;

	void MovieClip_beginFill_1(FlashSpriteInstance* self, uint32_t rgb) const;

	void MovieClip_beginFill_2(FlashSpriteInstance* self, uint32_t rgb, int32_t alpha) const;

	bool MovieClip_beginGradientFill(FlashSpriteInstance* self, const std::string& fillType, const Array* colors, const Array* alphas, const Array* ratios, ActionObject* matrix) const;

	void MovieClip_clear(FlashSpriteInstance* self) const;

	Ref< FlashSpriteInstance > MovieClip_createEmptyMovieClip(FlashSpriteInstance* self, const std::string& emptyClipName, int32_t depth) const;

	Ref< FlashEditInstance > MovieClip_createTextField(
		FlashSpriteInstance* self,
		const std::string& name,
		int32_t depth,
		avm_number_t x,
		avm_number_t y,
		avm_number_t width,
		avm_number_t height
	) const;

	void MovieClip_curveTo(FlashSpriteInstance* self, avm_number_t controlX, avm_number_t controlY, avm_number_t anchorX, avm_number_t anchorY) const;

	Ref< FlashSpriteInstance > MovieClip_duplicateMovieClip(FlashSpriteInstance* self, const std::string& name, int32_t depth) const;

	void MovieClip_endFill(FlashSpriteInstance* self) const;

	Ref< ActionObject > MovieClip_getBounds(FlashSpriteInstance* self, const FlashCharacterInstance* reference) const;

	int32_t MovieClip_getBytesLoaded(FlashSpriteInstance* self) const;

	int32_t MovieClip_getBytesTotal(FlashSpriteInstance* self) const;

	int32_t MovieClip_getDepth(FlashSpriteInstance* self) const;

	Ref< FlashCharacterInstance > MovieClip_getInstanceAtDepth(FlashSpriteInstance* self, int32_t depth) const;

	int32_t MovieClip_getNextHighestDepth(FlashSpriteInstance* self) const;

	Ref< ActionObject > MovieClip_getRect(FlashSpriteInstance* self, const FlashCharacterInstance* reference) const;

	int32_t MovieClip_getSWFVersion(FlashSpriteInstance* self) const;

	void MovieClip_getTextSnapshot(FlashSpriteInstance* self) const;

	void MovieClip_getURL(FlashSpriteInstance* self, const std::wstring& url) const;

	void MovieClip_globalToLocal(FlashSpriteInstance* self) const;

	void MovieClip_gotoAndPlay(FlashSpriteInstance* self, const ActionValue& arg0) const;

	void MovieClip_gotoAndStop(FlashSpriteInstance* self, const ActionValue& arg0) const;

	bool MovieClip_hitTest_1(const FlashSpriteInstance* self, const FlashCharacterInstance* shape) const;

	bool MovieClip_hitTest_2(const FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const;

	void MovieClip_lineGradientStyle(FlashSpriteInstance* self) const;

	void MovieClip_lineStyle(FlashSpriteInstance* self) const;

	void MovieClip_lineTo(FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const;

	Ref< FlashSpriteInstance > MovieClip_loadMovie(FlashSpriteInstance* self, const std::wstring& fileName) const;

	void MovieClip_loadVariables(FlashSpriteInstance* self) const;

	void MovieClip_localToGlobal(const FlashSpriteInstance* self) const;

	void MovieClip_moveTo(FlashSpriteInstance* self, avm_number_t x, avm_number_t y) const;

	void MovieClip_nextFrame(FlashSpriteInstance* self) const;

	void MovieClip_play(FlashSpriteInstance* self) const;

	void MovieClip_prevFrame(FlashSpriteInstance* self) const;

	void MovieClip_removeMovieClip(FlashSpriteInstance* self) const;

	void MovieClip_setMask(FlashSpriteInstance* self, FlashSpriteInstance* mask) const;

	void MovieClip_startDrag(FlashSpriteInstance* self) const;

	void MovieClip_stop(FlashSpriteInstance* self) const;

	void MovieClip_stopDrag(FlashSpriteInstance* self) const;

	void MovieClip_swapDepths(FlashSpriteInstance* self, const ActionValue& arg0) const;

	std::string MovieClip_toString(const FlashSpriteInstance* self) const;

	void MovieClip_unloadMovie(FlashSpriteInstance* self) const;

	avm_number_t MovieClip_get_alpha(const FlashSpriteInstance* self) const;

	void MovieClip_set_alpha(FlashSpriteInstance* self, avm_number_t alpha) const;

	void MovieClip_get_blendMode(const FlashSpriteInstance* self) const;

	void MovieClip_set_blendMode(FlashSpriteInstance* self) const;

	void MovieClip_get_cacheAsBitmap(const FlashSpriteInstance* self) const;

	void MovieClip_set_cacheAsBitmap(FlashSpriteInstance* self) const;

	int32_t MovieClip_get_currentframe(const FlashSpriteInstance* self) const;

	void MovieClip_get_droptarget(const FlashSpriteInstance* self) const;

	bool MovieClip_get_enabled(const FlashSpriteInstance* self) const;

	void MovieClip_set_enabled(FlashSpriteInstance* self, bool enabled) const;

	void MovieClip_get_filters(const FlashSpriteInstance* self) const;

	void MovieClip_set_filters(FlashSpriteInstance* self) const;

	void MovieClip_get_focusEnabled(const FlashSpriteInstance* self) const;

	void MovieClip_set_focusEnabled(FlashSpriteInstance* self) const;

	void MovieClip_get_focusrect(const FlashSpriteInstance* self) const;

	void MovieClip_set_focusrect(FlashSpriteInstance* self) const;

	void MovieClip_get_forceSmoothing(const FlashSpriteInstance* self) const;

	void MovieClip_set_forceSmoothing(FlashSpriteInstance* self) const;

	int32_t MovieClip_get_framesloaded(const FlashSpriteInstance* self) const;

	float MovieClip_get_height(const FlashSpriteInstance* self) const;

	void MovieClip_set_height(FlashSpriteInstance* self, float height) const;

	void MovieClip_get_highquality(const FlashSpriteInstance* self) const;

	void MovieClip_set_highquality(FlashSpriteInstance* self) const;

	void MovieClip_get_hitArea(const FlashSpriteInstance* self) const;

	void MovieClip_set_hitArea(FlashSpriteInstance* self) const;

	void MovieClip_get_lockroot(const FlashSpriteInstance* self) const;

	void MovieClip_set_lockroot(FlashSpriteInstance* self) const;

	void MovieClip_get_menu(const FlashSpriteInstance* self) const;

	void MovieClip_set_menu(FlashSpriteInstance* self) const;

	ActionValue MovieClip_get_name(const FlashSpriteInstance* self) const;

	void MovieClip_set_name(FlashSpriteInstance* self, const std::string& name) const;

	void MovieClip_get_opaqueBackground(const FlashSpriteInstance* self) const;

	void MovieClip_set_opaqueBackground(FlashSpriteInstance* self) const;

	FlashCharacterInstance* MovieClip_get_parent(FlashSpriteInstance* self) const;

	void MovieClip_set_parent(FlashSpriteInstance* self) const;

	std::string MovieClip_get_quality(const FlashSpriteInstance* self) const;

	void MovieClip_set_quality(FlashSpriteInstance* self, const std::string& quality) const;

	float MovieClip_get_rotation(const FlashSpriteInstance* self) const;

	void MovieClip_set_rotation(FlashSpriteInstance* self, float rotation) const;

	void MovieClip_get_scale9Grid(const FlashSpriteInstance* self) const;

	void MovieClip_set_scale9Grid(FlashSpriteInstance* self) const;

	void MovieClip_get_scrollRect(const FlashSpriteInstance* self) const;

	void MovieClip_set_scrollRect(FlashSpriteInstance* self) const;

	void MovieClip_get_soundbuftime(const FlashSpriteInstance* self) const;

	void MovieClip_set_soundbuftime(FlashSpriteInstance* self) const;

	void MovieClip_get_tabChildren(const FlashSpriteInstance* self) const;

	void MovieClip_set_tabChildren(FlashSpriteInstance* self) const;

	void MovieClip_get_tabEnabled(const FlashSpriteInstance* self) const;

	void MovieClip_set_tabEnabled(FlashSpriteInstance* self) const;

	void MovieClip_get_tabIndex(const FlashSpriteInstance* self) const;

	void MovieClip_set_tabIndex(FlashSpriteInstance* self) const;

	std::string MovieClip_get_target(const FlashSpriteInstance* self) const;

	uint32_t MovieClip_get_totalframes(const FlashSpriteInstance* self) const;

	void MovieClip_get_trackAsMenu(const FlashSpriteInstance* self) const;

	void MovieClip_set_trackAsMenu(FlashSpriteInstance* self) const;

	Ref< Transform > MovieClip_get_transform(FlashSpriteInstance* self) const;

	std::string MovieClip_get_url(const FlashSpriteInstance* self) const;

	void MovieClip_get_useHandCursor(const FlashSpriteInstance* self) const;

	void MovieClip_set_useHandCursor(FlashSpriteInstance* self) const;

	bool MovieClip_get_visible(const FlashSpriteInstance* self) const;

	void MovieClip_set_visible(FlashSpriteInstance* self, bool visible) const;

	float MovieClip_get_width(const FlashSpriteInstance* self) const;

	void MovieClip_set_width(FlashSpriteInstance* self, float width) const;

	float MovieClip_get_x(const FlashSpriteInstance* self) const;

	void MovieClip_set_x(FlashSpriteInstance* self, float x) const;

	int32_t MovieClip_get_xmouse(const FlashSpriteInstance* self) const;

	float MovieClip_get_xscale(const FlashSpriteInstance* self) const;

	void MovieClip_set_xscale(FlashSpriteInstance* self, float x) const;

	float MovieClip_get_y(const FlashSpriteInstance* self) const;

	void MovieClip_set_y(FlashSpriteInstance* self, float y) const;

	int32_t MovieClip_get_ymouse(const FlashSpriteInstance* self) const;

	float MovieClip_get_yscale(const FlashSpriteInstance* self) const;

	void MovieClip_set_yscale(FlashSpriteInstance* self, float y) const;
};

	}
}

#endif	// traktor_flash_AsMovieClip_H
