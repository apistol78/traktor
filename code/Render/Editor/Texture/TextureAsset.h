#ifndef traktor_render_TextureAsset_H
#define traktor_render_TextureAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	bool m_generateNormalMap;
	float m_scaleDepth;
	bool m_generateMips;
	bool m_keepZeroAlpha;
	bool m_isCubeMap;
	bool m_hasAlpha;
	bool m_ignoreAlpha;
	bool m_scaleImage;
	int32_t m_scaleWidth;
	int32_t m_scaleHeight;
	bool m_enableCompression;
	bool m_enableNormalMapCompression;
	bool m_inverseNormalMapY;
	bool m_linearGamma;
	bool m_generateSphereMap;

	TextureAsset();

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_render_TextureAsset_H
