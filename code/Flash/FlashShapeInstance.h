#ifndef traktor_flash_FlashShapeInstance_H
#define traktor_flash_FlashShapeInstance_H

#include "Flash/FlashCharacterInstance.h"

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

class FlashShape;

/*! \brief Flash shape instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashShapeInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashShapeInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashShape* shape);

	const FlashShape* getShape() const;

	virtual Aabb2 getBounds() const;

private:
	Ref< const FlashShape > m_shape;
};

	}
}

#endif	// traktor_flash_FlashShapeInstance_H
