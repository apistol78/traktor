#ifndef traktor_render_TextureOutput_H
#define traktor_render_TextureOutput_H

#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

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

class T_DLLCLASS TextureOutput : public ISerializable
{
	T_RTTI_CLASS;

public:
	TextureFormat m_textureFormat;
	bool m_generateNormalMap;
	float m_scaleDepth;
	bool m_generateMips;
	bool m_keepZeroAlpha;
	TextureType m_textureType;
	bool m_hasAlpha;
	bool m_ignoreAlpha;
	bool m_premultiplyAlpha;
	bool m_scaleImage;
	int32_t m_scaleWidth;
	int32_t m_scaleHeight;
	bool m_flipX;
	bool m_flipY;
	bool m_enableCompression;
	bool m_enableNormalMapCompression;
	bool m_inverseNormalMapY;
	bool m_linearGamma;
	bool m_generateSphereMap;
	bool m_preserveAlphaCoverage;
	float m_alphaCoverageReference;

	TextureOutput();

	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_render_TextureOutput_H
