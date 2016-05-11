#ifndef traktor_ui_BitmapWin32_H
#define traktor_ui_BitmapWin32_H

#define _WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#if defined(T_USE_GDI_PLUS)
// \hack As we don't want min/max defined but gdiplus.h require them
// we temporarily define them first.
#	define max(a,b) (((a) > (b)) ? (a) : (b))
#	define min(a,b) (((a) < (b)) ? (a) : (b))
#	include <gdiplus.h>
#	undef min
#	undef max
#endif
#include "Core/Misc/AutoPtr.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class BitmapWin32 : public ISystemBitmap
{
public:
	BitmapWin32();

	virtual bool create(uint32_t width, uint32_t height);

	virtual void destroy();

	virtual void copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos);

	virtual Ref< drawing::Image > getImage() const;

	virtual Size getSize() const;

	virtual void setPixel(uint32_t x, uint32_t y, const Color4ub& color);

	virtual Color4ub getPixel(uint32_t x, uint32_t y) const;

	LPVOID getBits() const { return m_pBits; }

	LPVOID getBitsPerMulAlpha() const { return m_pBitsPreMulAlpha; }

	COLORREF getMask() const { return m_mask; }

	HBITMAP getHBitmap() const { return m_hBitmap; }

	HBITMAP getHBitmapPreMulAlpha() const { return m_hBitmapPreMulAlpha; }

	HICON createIcon() const;

	HBITMAP createClone() const;

#if defined(T_USE_GDI_PLUS)
	Gdiplus::Bitmap* getGdiPlusBitmap();
#endif

private:
	HBITMAP m_hBitmap;
	HBITMAP m_hBitmapPreMulAlpha;
	LPVOID m_pBits;
	LPVOID m_pBitsPreMulAlpha;
	uint32_t m_width;
	uint32_t m_height;
	COLORREF m_mask;
#if defined(T_USE_GDI_PLUS)
	AutoPtr< Gdiplus::Bitmap > m_gpBitmap;
	AutoArrayPtr< uint32_t > m_gpBits;
	uint32_t m_gpAlphaAdd;
#endif
};

	}
}

#endif	// traktor_ui_BitmapWin32_H
