#ifndef traktor_spray_Source_H
#define traktor_spray_Source_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Transform.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

struct EmitterUpdateContext;
class EmitterInstance;

/*! \brief Particle source.
 * \ingroup Spray
 */
class T_DLLCLASS Source : public ISerializable
{
	T_RTTI_CLASS;

public:
	Source();

	virtual bool create(resource::IResourceManager* resourceManager) = 0;

	virtual void emit(
		EmitterUpdateContext& context,
		const Transform& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const = 0;

	virtual bool serialize(ISerializer& s);

	inline float getRate() const { return m_rate; }

private:
	float m_rate;
};

	}
}

#endif	// traktor_spray_Source_H
