/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ITexture_H
#define traktor_render_ITexture_H

#include "Core/Object.h"

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

/*! \brief Texture base class.
 * \ingroup Render
 */
class T_DLLCLASS ITexture : public Object
{
	T_RTTI_CLASS;
	
public:
	struct Lock
	{
		size_t pitch;
		void* bits;
	};

	virtual void destroy() = 0;

	virtual ITexture* resolve() = 0;
};
	
	}
}

#endif	// traktor_render_ITexture_H
