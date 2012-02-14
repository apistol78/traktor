#ifndef traktor_render_TextureAssetBase_H
#define traktor_render_TextureAssetBase_H

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
	namespace drawing
	{

class Image;

	}

	namespace render
	{

class T_DLLCLASS TextureAssetBase : public editor::Asset
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

	TextureAssetBase();

	virtual Ref< drawing::Image > load(const std::wstring& assetPath) const = 0;

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_render_TextureAssetBase_H
