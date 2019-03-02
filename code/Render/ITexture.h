#pragma once

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

	/*! \brief Destroy texture.
	 */
	virtual void destroy() = 0;

	/*! \brief Resolve render texture.
	 *
	 * \return Render texture.
	 */
	virtual ITexture* resolve() = 0;

	/*! \brief Get number of mip maps.
	 *
	 * \return Number of mips.
	 */
	virtual int32_t getMips() const = 0;
};

	}
}
