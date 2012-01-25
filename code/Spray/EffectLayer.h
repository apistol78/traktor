#ifndef traktor_spray_EffectLayer_H
#define traktor_spray_EffectLayer_H

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

class EffectLayerInstance;
class Emitter;
class Sequence;

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	EffectLayer();

	bool bind(resource::IResourceManager* resourceManager);

	Ref< EffectLayerInstance > createInstance() const;

	virtual bool serialize(ISerializer& s);

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	Emitter* getEmitter() const { return m_emitter; }

	Sequence* getSequence() const { return m_sequence; }

private:
	float m_time;
	float m_duration;
	Ref< Emitter > m_emitter;
	Ref< Sequence > m_sequence;
};

	}
}

#endif	// traktor_spray_EffectLayer_H
