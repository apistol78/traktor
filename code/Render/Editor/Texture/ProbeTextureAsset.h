/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProbeTextureAsset_H
#define traktor_render_ProbeTextureAsset_H

#include "Editor/Asset.h"
#include "Render/Editor/Texture/TextureOutput.h"

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

class T_DLLCLASS ProbeTextureAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	ProbeTextureAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_filterAngle;
	float m_filterMipDeltaAngle;

	friend class ProbeTexturePipeline;
};

	}
}

#endif	// traktor_render_ProbeTextureAsset_H
