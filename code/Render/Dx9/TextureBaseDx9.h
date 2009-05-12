#ifndef traktor_render_TextureBaseDx9_H
#define traktor_render_TextureBaseDx9_H

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX9 Xbox360
 */
class TextureBaseDx9
{
public:
	virtual IDirect3DBaseTexture9* getD3DBaseTexture() const = 0;
};

	}
}

#endif	// traktor_render_TextureBaseDx9_H
