/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IVolumeTexture_H
#define traktor_render_IVolumeTexture_H

#include "Render/ITexture.h"

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

/*! \brief Volume texture.
 * \ingroup Render
 */
class T_DLLCLASS IVolumeTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	virtual int getWidth() const = 0;

	virtual int getHeight() const = 0;

	virtual int getDepth() const = 0;
};
	
	}
}

#endif	// traktor_render_IVolumeTexture_H
