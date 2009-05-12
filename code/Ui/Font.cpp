#include "Ui/Font.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Font", Font, Object)

Font::Font()
:	m_size(0)
,	m_bold(false)
,	m_italic(false)
,	m_underline(false)
{
}

Font::Font(const Font& font)
:	m_face(font.m_face)
,	m_size(font.m_size)
,	m_bold(font.m_bold)
,	m_italic(font.m_italic)
,	m_underline(font.m_underline)
{
}

Font::Font(const std::wstring& face, int size, bool bold, bool italic, bool underline)
:	m_face(face)
,	m_size(size)
,	m_bold(bold)
,	m_italic(italic)
,	m_underline(underline)
{
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
	m_size = size;
}

int Font::getSize() const
{
	return m_size;
}

void Font::setBold(bool bold)
{
	m_bold = bold;
}

bool Font::isBold() const
{
	return m_bold;
}

void Font::setItalic(bool italic)
{
	m_italic = italic;
}

bool Font::isItalic() const
{
	return m_italic;
}

void Font::setUnderline(bool underline)
{
	m_underline = underline;
}

bool Font::isUnderline() const
{
	return m_underline;
}

bool Font::operator == (const Font& r) const
{
	return bool(m_size == r.m_size && m_bold == r.m_bold && m_italic == r.m_italic && m_underline == r.m_underline && m_face == r.m_face);
}

bool Font::operator != (const Font& r) const
{
	return !(*this == r);
}

bool Font::operator < (const Font& r) const
{
	if (m_size < r.m_size)
		return true;
	if (m_bold < r.m_bold)
		return true;
	if (m_italic < r.m_italic)
		return true;
	if (m_underline < r.m_underline)
		return true;
	if (m_face < r.m_face)
		return true;

	return false;
}

	}
}
