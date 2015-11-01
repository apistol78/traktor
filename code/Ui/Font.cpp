#include <cstring>
#include "Ui/Font.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Font", Font, Object)

Font::Font()
{
	m_params.size = 0;
	m_params.bold = false;
	m_params.italic = false;
	m_params.underline = false;
}

Font::Font(const Font& font)
:	m_face(font.m_face)
{
	m_params.size = font.m_params.size;
	m_params.bold = font.m_params.bold;
	m_params.italic = font.m_params.italic;
	m_params.underline = font.m_params.underline;
}

Font::Font(const std::wstring& face, int size, bool bold, bool italic, bool underline)
:	m_face(face)
{
	m_params.size = size;
	m_params.bold = bold;
	m_params.italic = italic;
	m_params.underline = underline;
}

void Font::setFace(const std::wstring& face)
{
	m_face = face;
}

std::wstring Font::getFace() const
{
	return m_face;
}

void Font::setSize(int size)
{
	m_params.size = size;
}

int Font::getSize() const
{
	return m_params.size;
}

int Font::getPointSize() const
{
	if (m_params.size > 0)
		return m_params.size;
	else
	{
		// Internal size specified in pixels, transform to points.
		return int((-m_params.size / 72.0f) * 96.0f);
	}
}

int Font::getPixelSize() const
{
	if (m_params.size > 0)
	{
		// Internal size specified in points, transform to pixels.
		return int((m_params.size / 96.0f) * 72.0f);
	}
	else
		return -m_params.size;
}

void Font::setBold(bool bold)
{
	m_params.bold = bold;
}

bool Font::isBold() const
{
	return m_params.bold;
}

void Font::setItalic(bool italic)
{
	m_params.italic = italic;
}

bool Font::isItalic() const
{
	return m_params.italic;
}

void Font::setUnderline(bool underline)
{
	m_params.underline = underline;
}

bool Font::isUnderline() const
{
	return m_params.underline;
}

bool Font::operator == (const Font& r) const
{
	if (m_face != r.m_face)
		return false;

	return std::memcmp(&m_params, &r.m_params, sizeof(m_params)) == 0;
}

bool Font::operator != (const Font& r) const
{
	return !(*this == r);
}

bool Font::operator < (const Font& r) const
{
	if (m_face < r.m_face)
		return true;

	if (std::memcmp(&m_params, &r.m_params, sizeof(m_params)) < 0)
		return true;

	return false;
}

	}
}
