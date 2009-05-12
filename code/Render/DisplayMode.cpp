#include "Render/DisplayMode.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DisplayMode", DisplayMode, Object)

DisplayMode::DisplayMode(int index, int width, int height, int colorBits) :
	m_index(index),
	m_width(width),
	m_height(height),
	m_colorBits(colorBits)
{
}

int DisplayMode::getIndex() const
{
	return m_index;
}

int DisplayMode::getWidth() const
{
	return m_width;
}

int DisplayMode::getHeight() const
{
	return m_height;
}

int DisplayMode::getColorBits() const
{
	return m_colorBits;
}

	}
}
