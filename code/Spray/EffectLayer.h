#ifndef traktor_spray_EffectLayer_H
#define traktor_spray_EffectLayer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class EffectLayerInstance;
class Emitter;
class ITrigger;
class Sequence;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayer : public Object
{
	T_RTTI_CLASS;

public:
	EffectLayer(
		float time,
		float duration,
		Emitter* emitter,
		Sequence* sequence,
		ITrigger* triggerEnable,
		ITrigger* triggerDisable
	);

	Ref< EffectLayerInstance > createInstance() const;

	float getTime() const { return m_time; }

	float getDuration() const { return m_duration; }

	const Emitter* getEmitter() const { return m_emitter; }

	const Sequence* getSequence() const { return m_sequence; }

	const ITrigger* getTriggerEnable() const { return m_triggerEnable; }

	const ITrigger* getTriggerDisable() const { return m_triggerDisable; }

private:
	float m_time;
	float m_duration;
	Ref< Emitter > m_emitter;
	Ref< Sequence > m_sequence;
	Ref< ITrigger > m_triggerEnable;
	Ref< ITrigger > m_triggerDisable;
};

	}
}

#endif	// traktor_spray_EffectLayer_H
