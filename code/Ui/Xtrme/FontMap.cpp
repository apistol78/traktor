#if defined(_WIN32)
#	define T_USE_GDI_FONTS
#endif
#if defined(T_USE_GDI_FONTS)
// Use GDI to render font map.
#	include <windows.h>
#	include <tchar.h>
#else
// Use FreeType to render font map.
#	include <ft2build.h>
#	include FT_FREETYPE_H
#	include "Core/Io/FileSystem.h"
#	include "Core/Io/Stream.h"
#endif

#include <cmath>
#include "Ui/Xtrme/FontMap.h"
#include "Ui/Font.h"
#include "Render/RenderSystem.h"
#include "Render/SimpleTexture.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{
		namespace xtrme
		{
			namespace
			{

inline int nearestPowerOf2(int value)
{
	int exp = 0;
	while ((1 << exp) < value)
		++exp;
	return (1 << exp);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.xtrme.FontMap", FontMap, Object)

void FontMap::create(render::RenderSystem* renderSystem, const Font& font)
{
#if defined(T_USE_GDI_FONTS)

	BITMAPINFO bmi;
	LOGFONT lf;

	HDC hdc = CreateCompatibleDC(NULL);
	T_ASSERT_M (hdc, L"Unable to create offscreen context");

	const int inch = GetDeviceCaps(hdc, LOGPIXELSY);

	int pixelSize = font.getSize() > 0 ? int(ceilf(((font.getSize() - 0.5f) * inch) / 72.0f)) : -font.getSize();
	int dim = nearestPowerOf2((pixelSize + 2) * 16);

	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = dim;
	bmi.bmiHeader.biHeight = dim;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	LPVOID bits = 0;
	HBITMAP hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	T_ASSERT_M (hbm, L"Unable to create DIB section");
	T_ASSERT_M (bits, L"No bits returned from CreateDIBSection");

	memset(bits, 0xff, dim * dim * 4);

	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = -pixelSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = font.isBold() ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = font.isItalic() ? TRUE : FALSE;
	lf.lfUnderline = font.isUnderline() ? TRUE : FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, LF_FACESIZE, wstots(font.getFace()).c_str());

	HFONT hfont = CreateFontIndirect(&lf);
	T_ASSERT_M (hfont, L"Unable to create device font");
	
	SelectObject(hdc, hbm);
	SelectObject(hdc, hfont);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0x00, 0x00, 0x00));

	for (int i = 0; i < 256; ++i)
	{
		int x = (i & 15) * (dim / 16);
		int y = (i / 16) * (dim / 16);

		char tmp[2] = { char(i), 0 };
		RECT rc = { x, y, 0, 0 };
		DrawText(
			hdc,
			mbstots(tmp).c_str(),
			1,
			&rc,
			DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX
		);

		SIZE size;
		GetTextExtentPoint32(hdc, mbstots(tmp).c_str(), 1, &size);

		m_glyphs[i].size[0] = size.cx; 
		m_glyphs[i].size[1] = size.cy;

		m_glyphs[i].uv[0] = float((i & 15) / 16.0f);
		m_glyphs[i].uv[1] = float((i / 16) / 16.0f);
		m_glyphs[i].uv[2] = m_glyphs[i].uv[0] + size.cx / float(dim);
		m_glyphs[i].uv[3] = m_glyphs[i].uv[1] + size.cy / float(dim);
	}

#if !defined(WINCE)
	uint32_t count = GetKerningPairs(hdc, 0, 0);
	if (count > 0)
	{
		std::vector< KERNINGPAIR > kp(count);
		GetKerningPairs(hdc, (DWORD)kp.size(), &kp[0]);

		for (uint32_t i = 0; i < count; ++i)
		{
			if (kp[i].iKernAmount == 0)
				continue;

			uint32_t key = (kp[i].wFirst << 16) | kp[i].wSecond;
			m_kerningPairs[key] = kp[i].iKernAmount;
		}
	}
#endif

	render::SimpleTextureCreateDesc desc;

	desc.width =
	desc.height = dim;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = false;

	Ref< render::SimpleTexture > texture = renderSystem->createSimpleTexture(desc);
	T_ASSERT_M (texture, L"Unable to create font texture");

	render::Texture::Lock lock;
	texture->lock(0, lock);

	uint8_t* dp = reinterpret_cast< uint8_t* >(lock.bits);
	uint8_t* sp = reinterpret_cast< uint8_t* >(bits);

	for (int y = 0; y < dim; ++y)
	{
		for (int x = 0; x < dim; ++x)
		{
			int r = sp[(x + (dim - y - 1) * dim) * 4 + 2];
			int g = sp[(x + (dim - y - 1) * dim) * 4 + 1];
			int b = sp[(x + (dim - y - 1) * dim) * 4 + 0];

			dp[(x + y * dim) * 4 + 0] = r;
			dp[(x + y * dim) * 4 + 1] = g;
			dp[(x + y * dim) * 4 + 2] = b;
			dp[(x + y * dim) * 4 + 3] = 255 - (r + g + b) / 3;
		}
	}

	texture->unlock(0);

	DeleteObject(hfont);
	DeleteObject(hbm);
	DeleteDC(hdc);

	m_texture = texture;

#else	// !T_USE_GDI_FONTS

	FT_Error err;
	FT_Library library;
	FT_Face face;

	// @hack Assume fonts is available in windows directory.
#if defined(_WIN32)
	Ref< Stream > stream = FileSystem::getInstance().open(L"C:/Windows/Fonts/" + font.getFace() + L".ttf", File::FmRead);
#elif defined(__APPLE__)
	Ref< Stream > stream = FileSystem::getInstance().open(L"/Library/Fonts/ArialHB.ttf", File::FmRead);
#else
	Ref< Stream > stream = FileSystem::getInstance().open(L"/usr/share/fonts/truetype/DejaVuSans.ttf", File::FmRead);
#endif
	T_ASSERT (stream);

	std::vector< uint8_t > fontBuffer(stream->available());
	stream->read(&fontBuffer[0], fontBuffer.size());
	stream->close();
	
	err = FT_Init_FreeType(&library);
	T_ASSERT (!err);

	err = FT_New_Memory_Face(library, (const FT_Byte*)&fontBuffer[0], fontBuffer.size(), 0, &face);
	T_ASSERT (!err);

	int pixelSize = abs(font.getSize());
	int dim = nearestPowerOf2((pixelSize + 2) * 16);

	FT_Set_Pixel_Sizes(face, 0, pixelSize);

	// Create font texture.
	render::SimpleTextureCreateDesc desc;

	desc.width =
	desc.height = dim;
	desc.mipCount = 1;
	desc.format = render::TfA8R8G8B8;
	desc.immutable = false;

	Ref< render::SimpleTexture > texture = renderSystem->createSimpleTexture(desc);
	T_ASSERT_M (texture, L"Unable to create font texture");

	render::Texture::Lock lock;
	texture->lock(0, lock);

	uint8_t* dp = reinterpret_cast< uint8_t* >(lock.bits);

	for (int i = 0; i < dim; ++i)
		memset(&dp[i * lock.pitch], 0, dim * 4);

	for (int i = 0; i < 256; ++i)
	{
		int x = (i & 15) * (dim / 16);
		int y = (i / 16) * (dim / 16);

		err = FT_Load_Char(face, char(i), FT_LOAD_RENDER);
		if (err)
			continue;

		for (int yy = 0; yy < face->glyph->bitmap.rows; ++yy)
		{
			for (int xx = 0; xx < face->glyph->bitmap.width; ++xx)
			{
				uint8_t* d = &dp[(x + xx) * 4 + (y + yy + (pixelSize - face->glyph->bitmap_top)) * lock.pitch];

				uint8_t g = face->glyph->bitmap.buffer ? face->glyph->bitmap.buffer[xx + yy * face->glyph->bitmap.pitch] : 0;
				d[0] = g;
				d[1] = g;
				d[2] = g;
				d[3] = g;
			}
		}

		int sizeX = face->glyph->advance.x >> 6;
		int sizeY = pixelSize;

		m_glyphs[i].size[0] = sizeX; 
		m_glyphs[i].size[1] = sizeY;

		m_glyphs[i].uv[0] = float((i & 15) / 16.0f);
		m_glyphs[i].uv[1] = float((i / 16) / 16.0f);
		m_glyphs[i].uv[2] = m_glyphs[i].uv[0] + sizeX / float(dim);
		m_glyphs[i].uv[3] = m_glyphs[i].uv[1] + sizeY / float(dim);
	}

	texture->unlock(0);

	m_texture = texture;

	FT_Done_FreeType(library);

#endif
}

int FontMap::getKerning(uint16_t charLeft, uint16_t charRight) const
{
	uint32_t key = (charLeft << 16) | charRight;
	std::map< uint32_t, int >::const_iterator i = m_kerningPairs.find(key);
	return i != m_kerningPairs.end() ? i->second : 0;
}

Size FontMap::getExtent(const std::wstring& text) const
{
	Size extent(0, 0);
	for (size_t i = 0; i < text.length(); ++i)
	{
		const FontMap::Glyph& glyph = m_glyphs[text[i]];

		int kerning = i > 0 ? getKerning(text[i - 1], text[i]) : 0;
		
		extent.cx += glyph.size[0] + kerning;
		extent.cy = std::max< int >(extent.cy, glyph.size[1]);
	}
	return extent;
}

		}
	}
}
