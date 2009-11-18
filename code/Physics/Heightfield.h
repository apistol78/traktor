#ifndef traktor_physics_Heightfield_H
#define traktor_physics_Heightfield_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace physics
	{

/*! \brief Heightfield geometry.
 * \ingroup Physics
 */
class T_DLLCLASS Heightfield : public Object
{
	T_RTTI_CLASS;

public:
	Heightfield(uint32_t size, const Vector4& worldExtent);

	virtual ~Heightfield();

	bool read(IStream* stream);

	bool write(IStream* stream);

	float getSampleBilinear(float x, float z) const;

	float getSampleTrilinear(float x, float z) const;

	inline uint32_t getSize() const { return m_size; }

	inline const Vector4& getWorldExtent() const { return m_worldExtent; }

	inline float* getHeights() const { return m_heights; }

private:
	uint32_t m_size;
	Vector4 m_worldExtent;
	float* m_heights;
};

	}
}

#endif	// traktor_physics_Heightfield_H
