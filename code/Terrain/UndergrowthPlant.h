#ifndef traktor_terrain_UndergrowthPlant_H
#define traktor_terrain_UndergrowthPlant_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace terrain
	{

class T_DLLCLASS UndergrowthPlant : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

	const std::vector< int32_t >& getPlants() const { return m_plants; }

private:
	std::vector< int32_t > m_plants;
};

	}
}

#endif	// traktor_terrain_UndergrowthPlant_H
