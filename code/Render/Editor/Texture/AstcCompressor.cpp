#include "Render/Editor/Texture/AstcCompressor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AstcCompressor", AstcCompressor, ICompressor)

bool AstcCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
    return false;
}

	}
}
