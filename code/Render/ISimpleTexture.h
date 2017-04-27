/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ISimpleTexture_H
#define traktor_render_ISimpleTexture_H

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

/*! \brief Simple 2d texture.
 * \ingroup Render
 */
class T_DLLCLASS ISimpleTexture : public ITexture
{
	T_RTTI_CLASS;
	
public:
	/*! \brief Get width in pixels of texture.
	 *
	 * \return Texture width in pixels.
	 */
	virtual int getWidth() const = 0;

	/*! \brief Get height in pixels of texture.
	 *
	 * \return Texture height in pixels.
	 */
	virtual int getHeight() const = 0;

	/*! \brief Lock access to texture data.
	 *
	 * \param level Mip level.
	 * \param lock Information about locked region.
	 * \return True if locked.
	 */
	virtual bool lock(int level, Lock& lock) = 0;

	/*! \brief Unlock access to texture data.
	 *
	 * \param level Mip level.
	 */
	virtual void unlock(int level) = 0;

	/*! \brief Get API specific internal handle.
	 *
	 * \return API handle to texture.
	 */
	virtual void* getInternalHandle() = 0;
};

	}
}

#endif	// traktor_render_ISimpleTexture_H
