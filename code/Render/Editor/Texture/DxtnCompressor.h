#ifndef traktor_render_DxtnCompressor_H
#define traktor_render_DxtnCompressor_H

#include "Render/Editor/Texture/ICompressor.h"

namespace traktor
{
	namespace render
	{

class DxtnCompressor : public ICompressor
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_DxtnCompressor_H
