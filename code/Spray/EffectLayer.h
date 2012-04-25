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
		Sequence* sequence
	);

	Ref< EffectLayerInstance > createInstance() const;

	float getTime() const { return m_time; }

	float getDuration() const { return m_duration; }

	const Emitter* getEmitter() const { return m_emitter; }

	const Sequence* getSequence() const { return m_sequence; }

private:
	float m_time;
	float m_duration;
	Ref< Emitter > m_emitter;
	Ref< Sequence > m_sequence;
};

	}
}

#endif	// traktor_spray_EffectLayer_H
