#pragma once

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

/*! Volume texture.
 * \ingroup Render
 */
class T_DLLCLASS IVolumeTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	/*! Get width in pixels of texture.
	 *
	 * \return Texture width in pixels.
	 */
	virtual int32_t getWidth() const = 0;

	/*! Get height in pixels of texture.
	 *
	 * \return Texture height in pixels.
	 */
	virtual int32_t getHeight() const = 0;

	/*! Get depth in pixels of texture.
	 *
	 * \return Texture depth in pixels.
	 */
	virtual int32_t getDepth() const = 0;

	/*! Get number of mip maps.
	 *
	 * \return Number of mips.
	 */
	virtual int32_t getMips() const = 0;	
};

	}
}
