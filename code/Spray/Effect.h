#ifndef traktor_spray_Effect_H
#define traktor_spray_Effect_H

#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Core/Serialization/Serializable.h"

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

class EffectLayer;
class EffectInstance;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS Effect : public Serializable
{
	T_RTTI_CLASS(Effect)

public:
	Effect();

	Ref< EffectInstance > createInstance(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(Serializer& s);

	inline float getDuration() const { return m_duration; }

	inline float getLoopStart() const { return m_loopStart; }

	inline float getLoopEnd() const { return m_loopEnd; }

	inline const RefArray< EffectLayer >& getLayers() const { return m_layers; }
	
private:
	float m_duration;
	float m_loopStart;
	float m_loopEnd;
	RefArray< EffectLayer > m_layers;
};

	}
}

#endif	// traktor_spray_Effect_H
