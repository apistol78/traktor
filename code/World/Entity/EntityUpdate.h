#ifndef traktor_world_EntityUpdate_H
#define traktor_world_EntityUpdate_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Entity update class.
 * \ingroup World
 *
 * Instance of this class is passed through Entity::update
 * calls.
 */
class T_DLLCLASS EntityUpdate : public Object
{
	T_RTTI_CLASS(EntityUpdate)

public:
	EntityUpdate(float deltaTime = 0.0f);

	inline void setDeltaTime(float deltaTime) { m_deltaTime = deltaTime; }

	inline float getDeltaTime() const { return m_deltaTime; }

private:
	float m_deltaTime;
};

	}
}

#endif	// traktor_world_EntityUpdate_H
