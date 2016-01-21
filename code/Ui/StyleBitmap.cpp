#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.StyleBitmap", StyleBitmap, IBitmap)

StyleBitmap::StyleBitmap(const wchar_t* const name)
:	m_name(name)
{
}

StyleBitmap::StyleBitmap(const wchar_t* const name, IBitmap* defaultBitmap)
:	m_name(name)
,	m_defaultBitmap(defaultBitmap)
{
}

StyleBitmap::StyleBitmap(const wchar_t* const name, const void* defaultBitmapResource, uint32_t defaultBitmapResourceSize)
:	m_name(name)
,	m_defaultBitmap(Bitmap::load(defaultBitmapResource, defaultBitmapResourceSize, L"image"))
{
}

void StyleBitmap::destroy()
{
	safeDestroy(m_bitmap);
}

Size StyleBitmap::getSize() const
{
	return resolve() ? m_bitmap->getSize() : Size();
}

Ref< drawing::Image > StyleBitmap::getImage() const
{
	return resolve() ? m_bitmap->getImage() : 0;
}

ISystemBitmap* StyleBitmap::getSystemBitmap() const
{
	return resolve() ? m_bitmap->getSystemBitmap() : 0;
}

bool StyleBitmap::resolve() const
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
		return false;

	std::wstring bmp = ss->getValue(m_name);
	if (bmp == m_path)
		return bool(m_bitmap != 0);

	safeDestroy(m_bitmap);

	if ((m_bitmap = Bitmap::load(bmp)) == 0)
		m_bitmap = m_defaultBitmap;

	m_path = bmp;

	return bool(m_bitmap != 0);
}

	}
}
