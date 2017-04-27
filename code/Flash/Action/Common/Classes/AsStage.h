/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsStage_H
#define traktor_flash_AsStage_H

#include "Core/Math/Vector4.h"
#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Stage class.
 * \ingroup Flash
 */
class AsStage : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsStage(ActionContext* context);

	void eventResize(int32_t width, int32_t height);

	/*! \brief Convert from screen coordinates to stage coordinates.
	 *
	 * \note
	 * Screen coordinates are expressing in Pixels and
	 * stage coordinates in Twips.
	 */
	Vector2 toStage(const Vector2& pos) const;

	/*! \brief Convert from stage coordinates to screen coordinates.
	 */
	Vector2 toScreen(const Vector2& pos) const;

	int32_t getViewWidth() const { return m_viewWidth; }

	int32_t getViewHeight() const { return m_viewHeight; }

	const Vector4& getFrameTransform() const { return m_frameTransform; }

	SwfAlignType getAlignH() const { return m_alignH; }

	SwfAlignType getAlignV() const { return m_alignV; }

	SwfScaleModeType getScaleMode() const { return m_scaleMode; }

private:
	int32_t m_width;
	int32_t m_height;
	int32_t m_viewWidth;
	int32_t m_viewHeight;
	SwfAlignType m_alignH;
	SwfAlignType m_alignV;
	SwfScaleModeType m_scaleMode;
	Vector4 m_frameTransform;

	void updateViewOffset();

	void Stage_get_align(CallArgs& ca);

	void Stage_set_align(CallArgs& ca);

	void Stage_get_height(CallArgs& ca);

	void Stage_get_scaleMode(CallArgs& ca);

	void Stage_set_scaleMode(CallArgs& ca);

	void Stage_get_showMenu(CallArgs& ca);

	void Stage_set_showMenu(CallArgs& ca);

	void Stage_get_width(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsStage_H
