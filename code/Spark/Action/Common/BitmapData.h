#pragma once

#include "Spark/Action/ActionObjectRelay.h"

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

class BitmapFilter;
class SpriteInstance;
class Point;
class Rectangle;

/*! \brief ActionScript boolean wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapData : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	BitmapData(int32_t width, int32_t height, bool transparent, uint32_t fillColor);

	BitmapData(drawing::Image* image);

	int32_t getWidth();

	int32_t getHeight();

	bool getTransparent();

	Ref< Rectangle > getRectangle();

	void applyFilter(const BitmapData* sourceBitmapData, const Rectangle* sourceRect, const Point* destPoint, const BitmapFilter* filter);

	void draw(SpriteInstance* source);

	void fillRect(const Rectangle* rectangle, uint32_t color);

	bool save(const std::wstring& fileName);

	drawing::Image* getImage() const;

private:
	Ref< drawing::Image > m_image;
};

	}
}

