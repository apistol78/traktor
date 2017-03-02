#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsStage.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsStage", AsStage, ActionObject)

AsStage::AsStage(ActionContext* context)
:	ActionObject(context)
,	m_width(0)
,	m_height(0)
,	m_viewWidth(0)
,	m_viewHeight(0)
,	m_alignH(SaCenter)
,	m_alignV(SaCenter)
,	m_scaleMode(SmShowAll)
,	m_frameTransform(0.0f, 0.0f, 0.0f, 0.0f)
{
	addProperty("align", createNativeFunction(context, this, &AsStage::Stage_get_align), createNativeFunction(context, this, &AsStage::Stage_set_align));
	addProperty("height", createNativeFunction(context, this, &AsStage::Stage_get_height), 0);
	addProperty("scaleMode", createNativeFunction(context, this, &AsStage::Stage_get_scaleMode), createNativeFunction(context, this, &AsStage::Stage_set_scaleMode));
	addProperty("showMenu", createNativeFunction(context, this, &AsStage::Stage_get_showMenu), createNativeFunction(context, this, &AsStage::Stage_set_showMenu));
	addProperty("width", createNativeFunction(context, this, &AsStage::Stage_get_width), 0);

	const FlashMovie* movie = context->getMovie();
	T_ASSERT (movie);

	m_width = int32_t((movie->getFrameBounds().mx.x - movie->getFrameBounds().mn.x) / 20.0f);
	m_height = int32_t((movie->getFrameBounds().mx.y - movie->getFrameBounds().mn.y) / 20.0f);

	m_viewWidth = m_width;
	m_viewHeight = m_height;

	updateViewOffset();
}

void AsStage::eventResize(int32_t width, int32_t height)
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

	ActionValue broadcastMessageValue;
	getMember("broadcastMessage", broadcastMessageValue);

	Ref< ActionFunction > broadcastMessageFn = broadcastMessageValue.getObject< ActionFunction >();
	if (broadcastMessageFn)
	{
		ActionValueArray args(getContext()->getPool(), 1);
		args[0] = ActionValue("onResize");
		broadcastMessageFn->call(this, args);
	}
}

Vector2 AsStage::toStage(const Vector2& pos) const
{
	const FlashMovie* movie = getContext()->getMovie();
	T_ASSERT (movie);

	Aabb2 bounds = movie->getFrameBounds();

	// Normalize screen coordinates into -1 to 1 ranges.
	float sx = 2.0f * pos.x / m_viewWidth - 1.0f;
	float sy = 2.0f * pos.y / m_viewHeight - 1.0f;

	// Inverse transform into stage coordinates.
	float tx = (((sx + 1.0f) / 2.0f - m_frameTransform.x()) / m_frameTransform.z()) * (bounds.mx.x - bounds.mn.x) + bounds.mn.x;
	float ty = (((sy + 1.0f) / 2.0f - m_frameTransform.y()) / m_frameTransform.w()) * (bounds.mx.y - bounds.mn.y) + bounds.mn.y;
	
	return Vector2(tx, ty);
}

Vector2 AsStage::toScreen(const Vector2& pos) const
{
	const FlashMovie* movie = getContext()->getMovie();
	T_ASSERT (movie);

	Aabb2 bounds = movie->getFrameBounds();

	// Normalize stage coordinates into 0 to 1 range.
	float tx = (pos.x - bounds.mn.x) / (bounds.mx.x - bounds.mn.x);
	float ty = (pos.y - bounds.mn.y) / (bounds.mx.y - bounds.mn.y);

	float vx = tx * m_frameTransform.z() + m_frameTransform.x();
	float vy = ty * m_frameTransform.w() + m_frameTransform.y();

	float sx = vx * m_viewWidth;
	float sy = vy * m_viewHeight;

	return Vector2(sx, sy);
}

void AsStage::updateViewOffset()
{
	m_frameTransform.set(0.0f, 0.0f, 1.0f, 1.0f);

	float aspectRatio = float(m_viewWidth) / m_viewHeight;
	if (aspectRatio <= FUZZY_EPSILON)
		return;

	const FlashMovie* movie = getContext()->getMovie();
	T_ASSERT (movie);

	Aabb2 bounds = movie->getFrameBounds();

	if (m_scaleMode == SmShowAll)
	{
		float frameAspect = (bounds.mx.x - bounds.mn.x) / (bounds.mx.y - bounds.mn.y);
		float scaleX = frameAspect / aspectRatio;
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
			float scaleY = 1.0f / scaleX;

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
		float frameAspect = (bounds.mx.x - bounds.mn.x) / (bounds.mx.y - bounds.mn.y);
		float scaleX = frameAspect / aspectRatio;
		if (scaleX <= 1.0f)
		{
			float scaleY = 1.0f / scaleX;

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
		float viewWidth = m_viewWidth * 20.0f;
		float viewHeight = m_viewHeight * 20.0f;

		float boundsWidth = (bounds.mx.x - bounds.mn.x);
		float boundsHeight = (bounds.mx.y - bounds.mn.y);

		float scaleX = boundsWidth / viewWidth;
		float scaleY = boundsHeight / viewHeight;

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

void AsStage::Stage_get_align(CallArgs& ca)
{
	const char* tbl[3][3] =
	{
		{ "TL", "T", "TR" },
		{ "L", "", "R" },
		{ "BR", "B", "BR" }
	};
	ca.ret = ActionValue(tbl[m_alignV][m_alignH]);
}

void AsStage::Stage_set_align(CallArgs& ca)
{
	std::string align = toUpper(ca.args[0].getString());
	if (align.size() >= 2)
	{
		if (align == "TL")
		{
			m_alignV = SaTop;
			m_alignH = SaLeft;
		}
		else if (align == "TR")
		{
			m_alignV = SaTop;
			m_alignH = SaRight;
		}
		else if (align == "BL")
		{
			m_alignV = SaBottom;
			m_alignH = SaLeft;
		}
		else if (align == "BR")
		{
			m_alignV = SaBottom;
			m_alignH = SaRight;
		}
	}
	else if (align.size() >= 1)
	{
		if (align == "T")
		{
			m_alignV = SaTop;
			m_alignH = SaCenter;
		}
		else if (align == "L")
		{
			m_alignV = SaCenter;
			m_alignH = SaLeft;
		}
		else if (align == "R")
		{
			m_alignV = SaCenter;
			m_alignH = SaRight;
		}
		else if (align == "B")
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

void AsStage::Stage_get_height(CallArgs& ca)
{
	ca.ret = ActionValue(m_height);
}

void AsStage::Stage_get_scaleMode(CallArgs& ca)
{
	const char* tbl[] =
	{
		"showAll",
		"noBorder",
		"exactFit",
		"noScale"
	};
	ca.ret = ActionValue(tbl[m_scaleMode]);
}

void AsStage::Stage_set_scaleMode(CallArgs& ca)
{
	std::string sm = ca.args[0].getString();

	if (compareIgnoreCase< std::string >(sm, "showAll") == 0)
		m_scaleMode = SmShowAll;
	else if (compareIgnoreCase< std::string >(sm, "noBorder") == 0)
		m_scaleMode = SmNoBorder;
	else if (compareIgnoreCase< std::string >(sm, "exactFit") == 0)
		m_scaleMode = SmExactFit;
	else if (compareIgnoreCase< std::string >(sm, "noScale") == 0)
	{
		m_scaleMode = SmNoScale;
		m_width = m_viewWidth;
		m_height = m_viewHeight;
	}

	updateViewOffset();
}

void AsStage::Stage_get_showMenu(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::get_showMenu not implemented" << Endl;
	)
}

void AsStage::Stage_set_showMenu(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"Stage::set_showMenu not implemented" << Endl;
	)
}

void AsStage::Stage_get_width(CallArgs& ca)
{
	ca.ret = ActionValue(m_width);
}

	}
}
