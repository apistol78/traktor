#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Spark/Bitmap.h"
#include "Spark/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace spark
	{

/*! \brief Flash bitmap container.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapImage : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapImage();

	BitmapImage(const drawing::Image* image);

	const void* getBits() const;

	const drawing::Image* getImage() const { return m_image; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< drawing::Image > m_image;
};

	}
}

