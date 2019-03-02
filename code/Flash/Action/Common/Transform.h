#pragma once

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
class CharacterInstance;

/*! \brief Geometry transform wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Transform : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Transform(CharacterInstance* instance);

	Ref< ColorTransform > getColorTransform() const;

	void setColorTransform(const ColorTransform* colorTransform);

protected:
	virtual void trace(visitor_t visitor) const override final;

	virtual void dereference() override final;

private:
	Ref< CharacterInstance > m_instance;
};

	}
}

