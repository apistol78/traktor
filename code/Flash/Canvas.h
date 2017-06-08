/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Canvas_H
#define traktor_flash_Canvas_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Dictionary.h"
#include "Flash/FillStyle.h"
#include "Flash/LineStyle.h"
#include "Flash/Path.h"
#include "Flash/Polygon.h"
#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Bitmap;

/*! \brief Dynamic canvas.
 * \ingroup Flash
 */
#pragma warning( disable:4324 )
class T_DLLCLASS Canvas : public Object
{
	T_RTTI_CLASS;

public:
	Canvas();

	int32_t getCacheTag() const;

	int32_t getDirtyTag() const;

	/*! \brief Clear canvas. */
	void clear();

	/*! \brief Begin solid fill. */
	void beginFill(const Color4f& color);

	/*! \brief Begin gradient fill. */
	void beginGradientFill(FillStyle::GradientType gradientType, const AlignedVector< FillStyle::ColorRecord >& colorRecords, const Matrix33& gradientMatrix);

	/*! \brief Begin bitmap fill. */
	void beginBitmapFill(Bitmap* image, const Matrix33& bitmapMatrix, bool repeat);

	/*! \brief End fill. */
	void endFill();

	/*! \brief Move cursor. */
	void moveTo(float x, float y);

	/*! \brief Add a line from cursor. */
	void lineTo(float x, float y);

	/*! \brief Add a curve from cursor. */
	void curveTo(float controlX, float controlY, float anchorX, float anchorY);

	/*! \brief Generate triangles and lines from canvas. */
	void triangulate(bool oddEven, AlignedVector< Triangle >& outTriangles, AlignedVector< Line >& outLines) const;

	/*! \brief Get bounds of shapes in canvas. */
	const Aabb2& getBounds() const { return m_bounds; }

	const Dictionary& getDictionary() const { return m_dictionary; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

private:
	int32_t m_cacheTag;
	int32_t m_dirtyTag;
	Aabb2 m_bounds;
	Dictionary m_dictionary;
	AlignedVector< Path > m_paths;
	AlignedVector< LineStyle > m_lineStyles;
	AlignedVector< FillStyle > m_fillStyles;
	bool m_drawing;
};

	}
}

#endif	// traktor_flash_Canvas_H
