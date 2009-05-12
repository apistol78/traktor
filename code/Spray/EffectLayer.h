#ifndef traktor_spray_EffectLayer_H
#define traktor_spray_EffectLayer_H

#include "Core/Heap/Ref.h"
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
	namespace spray
	{

class EffectLayerInstance;
class Emitter;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayer : public Serializable
{
	T_RTTI_CLASS(EffectLayer)

public:
	EffectLayer();

	EffectLayerInstance* createInstance() const;

	virtual bool serialize(Serializer& s);

	inline float getTime() const { return m_time; }

	inline float getDuration() const { return m_duration; }

	inline Emitter* getEmitter() const { return m_emitter; }

private:
	float m_time;
	float m_duration;
	Ref< Emitter > m_emitter;
};

	}
}

#endif	// traktor_spray_EffectLayer_H
