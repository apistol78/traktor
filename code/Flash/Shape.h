/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Shape_H
#define traktor_flash_Shape_H

#include "Flash/Path.h"
#include "Flash/Polygon.h"
#include "Flash/SwfTypes.h"
#include "Flash/Character.h"
#include "Flash/FillStyle.h"
#include "Flash/LineStyle.h"

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

#pragma warning( disable:4324 )

/*! \brief Flash shape.
 * \ingroup Flash
 */
class T_DLLCLASS Shape : public Character
{
	T_RTTI_CLASS;

public:
	Shape();

	Shape(uint16_t id);

	bool create(const Aabb2& shapeBounds, const SwfShape* shape, const SwfStyles* styles);

	bool create(const SwfShape* shape);

	bool create(uint16_t fillBitmap, int32_t width, int32_t height);

	void merge(const Shape& shape, const Matrix33& transform, const ColorTransform& cxform);

	void triangulate(bool oddEven, AlignedVector< Triangle >& outTriangles, AlignedVector< Line >& outLines) const;

	void triangulate(bool oddEven);

	void discardPaths();

	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< LineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< Triangle >& getTriangles() const { return m_triangles; }

	const AlignedVector< Line >& getLines() const { return m_lines; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_shapeBounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FillStyle > m_fillStyles;
	AlignedVector< LineStyle > m_lineStyles;
	AlignedVector< Triangle > m_triangles;
	AlignedVector< Line > m_lines;
};

#pragma warning( default:4324 )

	}
}

#endif	// traktor_flash_Shape_H
