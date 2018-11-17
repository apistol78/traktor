/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CubeMap_H
#define traktor_render_CubeMap_H

#include "Core/Object.h"
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;
class PixelFormat;

	}

	namespace render
	{

/*! \brief CubeMap wrapper.
 * \ingroup Render
 */
class T_DLLCLASS CubeMap : public Object
{
	T_RTTI_CLASS;

public:
	explicit CubeMap(int32_t size, const drawing::PixelFormat& pixelFormat);

	explicit CubeMap(const drawing::Image* cubeMap);

	/*! \brief Create flatten "cross" image. */
	Ref< drawing::Image > createCrossImage() const;

	/*! \brief Get normalized direction vector from coordinates. */
	Vector4 getDirection(int32_t side, int32_t x, int32_t y) const;

	/*! \brief Get coordinates from direction. */
	void getPosition(const Vector4& direction, int32_t& outSide, int32_t& outX, int32_t& outY) const;

	/*! \brief Set pixel value at direction. */
	void set(const Vector4& direction, const Color4f& value);

	/*! \brief Get pixel value from direction. */
	Color4f get(const Vector4& direction) const;

	/*! \brief Replace image of one side. */
	void setSide(int32_t side, drawing::Image* image) { m_side[side] = image; }

	/*! \brief Get image of one side. */
	drawing::Image* getSide(int32_t side) { return m_side[side]; }

	/*! \brief Get image of one side. */
	const drawing::Image* getSide(int32_t side) const { return m_side[side]; }

	/*! \brief Get size of cubemap. */
	int32_t getSize() const { return m_size; }

private:
	Ref< drawing::Image > m_side[6];
	int32_t m_size;
};

	}
}

#endif	// traktor_render_CubeMap_H
