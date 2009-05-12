#ifndef traktor_input_RumbleEffect_H
#define traktor_input_RumbleEffect_H

#include <vector>
#include "Core/Serialization/Serializable.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS RumbleEffect : public Serializable
{
	T_RTTI_CLASS(RumbleEffect)

public:
	void setRumbleKeys(const std::vector< std::pair< float, InputRumble > >& rumbleKeys);

	float getDuration() const;

	void getRumble(float time, InputRumble& outRumble) const;

	virtual bool serialize(Serializer& s);

private:
	std::vector< std::pair< float, InputRumble > > m_rumbleKeys;
};

	}
}

#endif	// traktor_input_RumbleEffect_H
