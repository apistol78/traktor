/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ICubeTexture_H
#define traktor_render_ICubeTexture_H

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

/*! \brief Cube texture.
 * \ingroup Render
 */
class T_DLLCLASS ICubeTexture : public ITexture
{
	T_RTTI_CLASS;
	
public:
	enum Side
	{
		SdPositiveX	= 0,
		SdNegativeX = 1,
		SdPositiveY = 2,
		SdNegativeY = 3,
		SdPositiveZ = 4,
		SdNegativeZ = 5
	};

	virtual int getWidth() const = 0;

	virtual int getHeight() const = 0;

	virtual int getDepth() const = 0;

	virtual bool lock(int side, int level, Lock& lock) = 0;

	virtual void unlock(int side, int level) = 0;
};
	
	}
}

#endif	// traktor_render_ICubeTexture_H
