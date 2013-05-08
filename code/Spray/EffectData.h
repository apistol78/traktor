#ifndef traktor_spray_EffectData_H
#define traktor_spray_EffectData_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class Effect;
class EffectLayerData;

/*! \brief Effect persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EffectData();

	EffectData(
		float duration,
		float loopStart,
		float loopEnd,
		const RefArray< EffectLayerData >& layers
	);

	Ref< Effect > createEffect(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	float getDuration() const { return m_duration; }

	float getLoopStart() const { return m_loopStart; }

	float getLoopEnd() const { return m_loopEnd; }

	const RefArray< EffectLayerData >& getLayers() const { return m_layers; }
	
private:
	float m_duration;
	float m_loopStart;
	float m_loopEnd;
	RefArray< EffectLayerData > m_layers;
};

	}
}

#endif	// traktor_spray_EffectData_H
