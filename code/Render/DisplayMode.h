#ifndef traktor_render_DisplayMode_H
#define traktor_render_DisplayMode_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Display mode descriptor.
 * \ingroup Render
 *
 * Information about one possible display mode.
 * Instances of this object is used to retrieve available modes on the host
 * system and are then used to pass information to the render system which
 * mode is desired.
 * As it's not possible to enumerate non-fullscreen modes it's required that
 * the user application doesn't expect to find any non-fullscreen mode when
 * determining best match.
 */
class T_DLLCLASS DisplayMode : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Initialize display mode.
	 *
	 * \param index Index of display mode.
	 * \param width Width of display.
	 * \param height Height of display.
	 * \param colorBits Number of color bits, usually 8, 16, 24 or 32.
	 */
	DisplayMode(int index, int width, int height, int colorBits);
	
	/*! \brief Display mode index.
	 *
	 * Get index of display mode. This index is very system specific
	 * and should not be used to identify the mode itself.
	 * \return Index of display mode.
	 */
	int getIndex() const;
	
	/*! \brief Display width.
	 *
	 * Return width of display mode.
	 * \return Number of pixels wide.
	 */
	int getWidth() const;
	
	/*! \brief Display height.
	 *
	 * Return height of display mode.
	 * \return Number of pixels high.
	 */
	int getHeight() const;

	/*! \brief Display color depth.
	 *
	 * Get number of bits the pixels are wide.
	 * \return Number of color bits.
	 */
	int getColorBits() const;
	
private:
	int m_index;
	int m_width;
	int m_height;
	int m_colorBits;
};

	}
}

#endif	// traktor_render_DisplayMode_H
