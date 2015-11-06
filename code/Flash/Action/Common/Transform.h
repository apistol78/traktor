#ifndef traktor_flash_Transform_H
#define traktor_flash_Transform_H

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

class ColorTransform;
class FlashCharacterInstance;

/*! \brief Geometry transform wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Transform : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Transform(FlashCharacterInstance* instance);

	Ref< ColorTransform > getColorTransform() const;

	void setColorTransform(const ColorTransform* colorTransform);

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE T_FINAL;

	virtual void dereference() T_OVERRIDE T_FINAL;

private:
	Ref< FlashCharacterInstance > m_instance;
};

	}
}

#endif	// traktor_flash_Transform_H
