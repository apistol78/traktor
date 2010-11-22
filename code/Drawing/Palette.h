#ifndef traktor_drawing_Palette_H
#define traktor_drawing_Palette_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	Palette(int size = 256);

	int getSize() const;
	
	void set(int index, const Color4f& c);
	
	const Color4f& get(int index) const;
	
	int find(const Color4f& c, bool exact = false) const;
	
private:
	AlignedVector< Color4f > m_colors;
};

	}
}

#endif	// traktor_drawing_Palette_H
