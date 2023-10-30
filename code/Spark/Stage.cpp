/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Spark/Context.h"
#include "Spark/Movie.h"
#include "Spark/Stage.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Stage", Stage, Object)

Stage::Stage(Context* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_viewWidth(0)
,	m_viewHeight(0)
,	m_alignH(SaCenter)
,	m_alignV(SaCenter)
,	m_scaleMode(SmShowAll)
,	m_frameTransform(0.0f, 0.0f, 0.0f, 0.0f)
{
	const Movie* movie = m_context->getMovie();
	T_ASSERT(movie);

	m_width = int32_t((movie->getFrameBounds().mx.x - movie->getFrameBounds().mn.x) / 20.0f);
	m_height = int32_t((movie->getFrameBounds().mx.y - movie->getFrameBounds().mn.y) / 20.0f);

	m_viewWidth = m_width;
	m_viewHeight = m_height;

	updateViewOffset();
}

void Stage::eventResize(int32_t width, int32_t height)
{
	m_viewWidth = width;
	m_viewHeight = height;

	// Only adjust stage's size when in NoScale mode.
	if (
		m_scaleMode != SmNoScale ||
		(width == m_width && height == m_height)
	)
	{
		updateViewOffset();
		return;
	}

	m_width = width;
	m_height = height;

	updateViewOffset();

	m_eventResize.issue();
}

Vector2 Stage::toStage(const Vector2& pos) const
{
	const Movie* movie = m_context->getMovie();
	T_ASSERT(movie);

	const Aabb2 bounds = movie->getFrameBounds();

	// Normalize screen coordinates into -1 to 1 ranges.
	const float sx = 2.0f * pos.x / m_viewWidth - 1.0f;
	const float sy = 2.0f * pos.y / m_viewHeight - 1.0f;

	// Inverse transform into stage coordinates.
	const float tx = (((sx + 1.0f) / 2.0f - m_frameTransform.x()) / m_frameTransform.z()) * (bounds.mx.x - bounds.mn.x) + bounds.mn.x;
	const float ty = (((sy + 1.0f) / 2.0f - m_frameTransform.y()) / m_frameTransform.w()) * (bounds.mx.y - bounds.mn.y) + bounds.mn.y;

	return Vector2(tx, ty);
}

Vector2 Stage::toScreen(const Vector2& pos) const
{
	const Movie* movie = m_context->getMovie();
	T_ASSERT(movie);

	const Aabb2 bounds = movie->getFrameBounds();

	// Normalize stage coordinates into 0 to 1 range.
	const float tx = (pos.x - bounds.mn.x) / (bounds.mx.x - bounds.mn.x);
	const float ty = (pos.y - bounds.mn.y) / (bounds.mx.y - bounds.mn.y);

	const float vx = tx * m_frameTransform.z() + m_frameTransform.x();
	const float vy = ty * m_frameTransform.w() + m_frameTransform.y();

	const float sx = vx * m_viewWidth;
	const float sy = vy * m_viewHeight;

	return Vector2(sx, sy);
}

void Stage::setAlign(const std::wstring& align_)
{
	const std::wstring align = toUpper(align_);
	if (align.size() >= 2)
	{
		if (align == L"TL")
		{
			m_alignV = SaTop;
			m_alignH = SaLeft;
		}
		else if (align == L"TR")
		{
			m_alignV = SaTop;
			m_alignH = SaRight;
		}
		else if (align == L"BL")
		{
			m_alignV = SaBottom;
			m_alignH = SaLeft;
		}
		else if (align == L"BR")
		{
			m_alignV = SaBottom;
			m_alignH = SaRight;
		}
	}
	else if (align.size() >= 1)
	{
		if (align == L"T")
		{
			m_alignV = SaTop;
			m_alignH = SaCenter;
		}
		else if (align == L"L")
		{
			m_alignV = SaCenter;
			m_alignH = SaLeft;
		}
		else if (align == L"R")
		{
			m_alignV = SaCenter;
			m_alignH = SaRight;
		}
		else if (align == L"B")
		{
			m_alignV = SaBottom;
			m_alignH = SaCenter;
		}
	}
	else
	{
		m_alignV = SaCenter;
		m_alignH = SaCenter;
	}

	updateViewOffset();
}

std::wstring Stage::getAlign() const
{
	const wchar_t* tbl[3][3] =
	{
		{ L"TL", L"T", L"TR" },
		{ L"L",  L"",  L"R" },
		{ L"BR", L"B", L"BR" }
	};
	return tbl[m_alignV][m_alignH];
}

void Stage::updateViewOffset()
{
	m_frameTransform.set(0.0f, 0.0f, 1.0f, 1.0f);

	const float aspectRatio = float(m_viewWidth) / m_viewHeight;
	if (aspectRatio <= FUZZY_EPSILON)
		return;

	const Movie* movie = m_context->getMovie();
	T_ASSERT(movie);

	const Aabb2 bounds = movie->getFrameBounds();

	if (m_scaleMode == SmShowAll)
	{
		const float frameAspect = (bounds.mx.x - bounds.mn.x) / (bounds.mx.y - bounds.mn.y);
		const float scaleX = frameAspect / aspectRatio;
		if (scaleX <= 1.0f)
		{
			float leftX = 0.0f;
			switch (m_alignH)
			{
			case SaLeft:
				leftX = 0.0f;
				break;
			case SaCenter:
				leftX = -(scaleX - 1.0f) / 2.0f;
				break;
			case SaRight:
				leftX = -(scaleX - 1.0f);
				break;
			}

			m_frameTransform.set(leftX, 0.0f, scaleX, 1.0f);
		}
		else
		{
			const float scaleY = 1.0f / scaleX;

			float topY = 0.0f;
			switch (m_alignV)
			{
			case SaTop:
				topY = 0.0f;
				break;
			case SaCenter:
				topY = -(scaleY - 1.0f) / 2.0f;
				break;
			case SaBottom:
				topY = -(scaleY - 1.0f);
				break;
			}

			m_frameTransform.set(0.0f, topY, 1.0f, scaleY);
		}
	}
	else if (m_scaleMode == SmNoBorder)
	{
		const float frameAspect = (bounds.mx.x - bounds.mn.x) / (bounds.mx.y - bounds.mn.y);
		const float scaleX = frameAspect / aspectRatio;
		if (scaleX <= 1.0f)
		{
			const float scaleY = 1.0f / scaleX;

			float topY = 0.0f;
			switch (m_alignV)
			{
			case SaTop:
				topY = 0.0f;
				break;
			case SaCenter:
				topY = -(scaleY - 1.0f) / 2.0f;
				break;
			case SaBottom:
				topY = -(scaleY - 1.0f);
				break;
			}

			m_frameTransform.set(0.0f, topY, 1.0f, scaleY);
		}
		else
		{
			float leftX = 0.0f;
			switch (m_alignH)
			{
			case SaLeft:
				leftX = 0.0f;
				break;
			case SaCenter:
				leftX = -(scaleX - 1.0f) / 2.0f;
				break;
			case SaRight:
				leftX = -(scaleX - 1.0f);
				break;
			}

			m_frameTransform.set(leftX, 0.0f, scaleX, 1.0f);
		}
	}
	else if (m_scaleMode == SmNoScale)
	{
		const float viewWidth = m_viewWidth * 20.0f;
		const float viewHeight = m_viewHeight * 20.0f;

		const float boundsWidth = (bounds.mx.x - bounds.mn.x);
		const float boundsHeight = (bounds.mx.y - bounds.mn.y);

		const float scaleX = boundsWidth / viewWidth;
		const float scaleY = boundsHeight / viewHeight;

		float leftX = 0.0f, topY = 0.0f;
		switch (m_alignH)
		{
		case SaLeft:
			leftX = 0.0f;
			break;
		case SaCenter:
			leftX = (viewWidth - boundsWidth) / 2.0f;
			break;
		case SaRight:
			leftX = viewWidth - boundsWidth;
			break;
		}

		switch (m_alignV)
		{
		case SaTop:
			topY = 0.0f;
			break;
		case SaCenter:
			topY = (viewHeight - boundsHeight) / 2.0f;
			break;
		case SaRight:
			topY = viewHeight - boundsHeight;
			break;
		}

		m_frameTransform.set(leftX / viewWidth, topY / viewHeight, scaleX, scaleY);
	}
}

void Stage::setScaleMode(const std::wstring& scaleMode)
{
	if (compareIgnoreCase(scaleMode, L"showAll") == 0)
		m_scaleMode = SmShowAll;
	else if (compareIgnoreCase(scaleMode, L"noBorder") == 0)
		m_scaleMode = SmNoBorder;
	else if (compareIgnoreCase(scaleMode, L"exactFit") == 0)
		m_scaleMode = SmExactFit;
	else if (compareIgnoreCase(scaleMode, L"noScale") == 0)
	{
		m_scaleMode = SmNoScale;
		m_width = m_viewWidth;
		m_height = m_viewHeight;
	}

	updateViewOffset();
}

std::wstring Stage::getScaleMode() const
{
	const wchar_t* tbl[] =
	{
		L"showAll",
		L"noBorder",
		L"exactFit",
		L"noScale"
	};
	return tbl[m_scaleMode];
}

}
