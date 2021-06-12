#pragma once

#include "Sound/IFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! Time stretch filter.
 * \ingroup Sound
 */
class T_DLLCLASS TimeStretchFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	TimeStretchFilter() = default;

	explicit TimeStretchFilter(float factor);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_factor = 1.0f;
};

	}
}
