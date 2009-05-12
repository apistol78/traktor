#ifndef traktor_ui_xtrme_FontMap_H
#define traktor_ui_xtrme_FontMap_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace render
	{

class RenderSystem;
class Texture;

	}

	namespace ui
	{

class Font;

		namespace xtrme
		{

class FontMap : public Object
{
	T_RTTI_CLASS(FontMapper)

public:
	struct Glyph
	{
		int size[2];
		float uv[4];
	};

	void create(render::RenderSystem* renderSystem, const Font& font);

	int getKerning(uint16_t charLeft, uint16_t charRight) const;

	Size getExtent(const std::wstring& text) const;

	inline const Glyph& getGlyph(wchar_t ch) const
	{
		return m_glyphs[ch & 255];
	}

	inline render::Texture* getTexture() const
	{
		return m_texture;
	}

private:
	Ref< render::Texture > m_texture;
	Glyph m_glyphs[256];
	std::map< uint32_t, int > m_kerningPairs;
};

		}
	}
}

#endif	// traktor_ui_xtrme_FontMap_H
