#ifndef traktor_spray_EffectLayerData_H
#define traktor_spray_EffectLayerData_H

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

class EffectLayer;
class EmitterData;
class SequenceData;

/*! \brief Effect layer persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EffectLayerData();

	Ref< EffectLayer > createEffectLayer(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	EmitterData* getEmitter() const { return m_emitter; }

	SequenceData* getSequence() const { return m_sequence; }

private:
	float m_time;
	float m_duration;
	Ref< EmitterData > m_emitter;
	Ref< SequenceData > m_sequence;
};

	}
}

#endif	// traktor_spray_EffectLayerData_H
