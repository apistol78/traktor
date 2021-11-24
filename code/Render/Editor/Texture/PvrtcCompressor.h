#pragma once

#include "Render/Editor/Texture/ICompressor.h"

namespace traktor
{
	namespace render
	{

/*! PVRTC texture compressor.
 * \ingroup Render
 */
class PvrtcCompressor : public ICompressor
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const override final;
};

	}
}

