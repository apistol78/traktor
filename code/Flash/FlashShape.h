#ifndef traktor_flash_FlashShape_H
#define traktor_flash_FlashShape_H

#include "Flash/Path.h"
#include "Flash/Polygon.h"
#include "Flash/SwfTypes.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/FlashLineStyle.h"

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

/*! \brief Flash shape.
 * \ingroup Flash
 */
class T_DLLCLASS FlashShape : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	FlashShape();

	FlashShape(uint16_t id);

	bool create(const Aabb2& shapeBounds, const SwfShape* shape, const SwfStyles* styles);

	bool create(const SwfShape* shape);

	bool create(uint16_t fillBitmap, int32_t width, int32_t height);

	void merge(const FlashShape& shape, const Matrix33& transform, const SwfCxTransform& cxform);

	void triangulate(bool oddEven);

	void discardPaths();

	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashDictionary* dictionary,
		FlashCharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	const Aabb2& getShapeBounds() const { return m_shapeBounds; }

	const AlignedVector< Path >& getPaths() const { return m_paths; }

	const AlignedVector< FlashFillStyle >& getFillStyles() const { return m_fillStyles; }

	const AlignedVector< FlashLineStyle >& getLineStyles() const { return m_lineStyles; }

	const AlignedVector< Triangle >& getTriangles() const { return m_triangles; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Aabb2 m_shapeBounds;
	AlignedVector< Path > m_paths;
	AlignedVector< FlashFillStyle > m_fillStyles;
	AlignedVector< FlashLineStyle > m_lineStyles;
	AlignedVector< Triangle > m_triangles;
};

	}
}

#endif	// traktor_flash_FlashShape_H
