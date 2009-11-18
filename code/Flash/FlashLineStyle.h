#ifndef traktor_flash_FlashLineStyle_H
#define traktor_flash_FlashLineStyle_H

#include "Core/Object.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief Shape line style.
 * \ingroup Flash
 */
class T_DLLCLASS FlashLineStyle : public Object
{
	T_RTTI_CLASS;

public:
	FlashLineStyle();

	bool create(const SwfLineStyle* lineStyle);

	const SwfColor& getLineColor() const;

	uint16_t getLineWidth() const;

private:
	SwfColor m_lineColor;
	uint16_t m_lineWidth;
};

	}
}

#endif	// traktor_flash_FlashLineStyle_H
