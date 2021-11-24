#pragma once

#include "Render/Editor/Texture/ICompressor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! ASTC texture compressor.
 * \ingroup Render
 */
class T_DLLCLASS AstcCompressor : public ICompressor
{
	T_RTTI_CLASS;

public:
	virtual bool compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const override final;
};

	}
}

