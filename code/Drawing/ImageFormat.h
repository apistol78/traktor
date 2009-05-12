#ifndef traktor_drawing_ImageFormat_H
#define traktor_drawing_ImageFormat_H

#include "Core/Io/Path.h"
#include "Core/Io/Stream.h"

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

class Image;

/*! \brief Image file format base class.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormat : public Object
{
	T_RTTI_CLASS(ImageFormat)

public:
	static ImageFormat* determineFormat(const std::wstring& extension);

	static ImageFormat* determineFormat(const Path& fileName);

	virtual Image* read(Stream* stream) = 0;

	virtual bool write(Stream* stream, Image* image) = 0;
};

	}
}

#endif	// traktor_drawing_ImageFormat_H
