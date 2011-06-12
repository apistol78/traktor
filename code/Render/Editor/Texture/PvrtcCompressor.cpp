#include "Render/Editor/Texture/PvrtcCompressor.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PvrtcCompressor", PvrtcCompressor, ICompressor)

bool PvrtcCompressor::compress(Writer& writer, const RefArray< drawing::Image >& mipImages, TextureFormat textureFormat, bool needAlpha, int32_t compressionQuality) const
{
	return false;
}

	}
}
