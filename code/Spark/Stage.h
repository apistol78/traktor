/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Spark/Event.h"
#include "Spark/Swf/SwfTypes.h"

namespace traktor::spark
{

/*! Stage class.
 * \ingroup Spark
 */
class Stage : public Object
{
	T_RTTI_CLASS;

public:
	explicit Stage(Context* context);

	void eventResize(int32_t width, int32_t height);

	/*! Convert from screen coordinates to stage coordinates.
	 *
	 * \note
	 * Screen coordinates are expressing in Pixels and
	 * stage coordinates in Twips.
	 */
	Vector2 toStage(const Vector2& pos) const;

	/*! Convert from stage coordinates to screen coordinates.
	 */
	Vector2 toScreen(const Vector2& pos) const;

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	int32_t getViewWidth() const { return m_viewWidth; }

	int32_t getViewHeight() const { return m_viewHeight; }

	const Vector4& getFrameTransform() const { return m_frameTransform; }

	SwfAlignType getAlignH() const { return m_alignH; }

	SwfAlignType getAlignV() const { return m_alignV; }

	void setAlign(const std::wstring& align);

	std::wstring getAlign() const;

	void setScaleMode(const std::wstring& scaleMode);

	std::wstring getScaleMode() const;

	Event* getEventResize() { return &m_eventResize; }

private:
	Context* m_context;
	int32_t m_width;
	int32_t m_height;
	int32_t m_viewWidth;
	int32_t m_viewHeight;
	SwfAlignType m_alignH;
	SwfAlignType m_alignV;
	SwfScaleModeType m_scaleMode;
	Vector4 m_frameTransform;
	Event m_eventResize;

	void updateViewOffset();
};

}
