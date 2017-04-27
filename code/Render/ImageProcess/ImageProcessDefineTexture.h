/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessDefineTexture_H
#define traktor_render_ImageProcessDefineTexture_H

#include "Render/Types.h"
#include "Resource/Id.h"
#include "Render/ImageProcess/ImageProcessDefine.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! \brief Define a texture parameter.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessDefineTexture : public ImageProcessDefine
{
	T_RTTI_CLASS;

public:
	virtual bool define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight) T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< ITexture >& getTexture() const { return m_texture; }

private:
	std::wstring m_parameter;
	resource::Id< ITexture > m_texture;
};

	}
}

#endif	// traktor_render_ImageProcessDefineTexture_H
