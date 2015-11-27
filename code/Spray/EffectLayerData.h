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

	namespace world
	{

class IEntityBuilder;
class IEntityEventData;

	}

	namespace spray
	{

class EffectLayer;
class EmitterData;
class SequenceData;
class TrailData;

/*! \brief Effect layer persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EffectLayerData();

	Ref< EffectLayer > createEffectLayer(resource::IResourceManager* resourceManager, const world::IEntityBuilder* entityBuilder) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	EmitterData* getEmitter() const { return m_emitter; }

	TrailData* getTrail() const { return m_trail; }

	SequenceData* getSequence() const { return m_sequence; }

	world::IEntityEventData* getTriggerEnable() const { return m_triggerEnable; }

	world::IEntityEventData* getTriggerDisable() const { return m_triggerDisable; }

private:
	std::wstring m_name;
	float m_time;
	float m_duration;
	Ref< EmitterData > m_emitter;
	Ref< TrailData > m_trail;
	Ref< SequenceData > m_sequence;
	Ref< world::IEntityEventData > m_triggerEnable;
	Ref< world::IEntityEventData > m_triggerDisable;
};

	}
}

#endif	// traktor_spray_EffectLayerData_H
