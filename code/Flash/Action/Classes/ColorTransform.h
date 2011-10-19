#ifndef traktor_flash_ColorTransform_H
#define traktor_flash_ColorTransform_H

#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionObjectRelay.h"

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

/*! \brief Color transform wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS ColorTransform : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	ColorTransform(const SwfCxTransform& transform);

	void setTransform(const SwfCxTransform& transform);

	SwfCxTransform& getTransform();

	const SwfCxTransform& getTransform() const;

private:
	SwfCxTransform m_transform;
};

	}
}

#endif	// traktor_flash_ColorTransform_H
