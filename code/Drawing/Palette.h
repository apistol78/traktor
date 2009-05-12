#ifndef traktor_drawing_Palette_H
#define traktor_drawing_Palette_H

#include <vector>
#include "Drawing/Color.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief Palette
 * \ingroup Drawing
 */
class T_DLLCLASS Palette : public Object
{
	T_RTTI_CLASS(Palette)

public:
	Palette(int size = 256);

	int getSize() const;
	
	void set(int index, const Color& c);
	
	const Color& get(int index) const;
	
	int find(const Color& c, bool exact = false) const;
	
private:
	std::vector< Color > m_colors;
};

	}
}

#endif	// traktor_drawing_Palette_H
