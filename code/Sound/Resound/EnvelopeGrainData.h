#ifndef traktor_sound_EnvelopeGrainData_H
#define traktor_sound_EnvelopeGrainData_H

#include "Core/RefArray.h"
#include "Sound/Resound/IGrainData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS EnvelopeGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	struct GrainData
	{
		Ref< IGrainData > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;

		GrainData()
		:	in(0.0f)
		,	out(0.0f)
		,	easeIn(0.2f)
		,	easeOut(0.2f)
		{
		}

		bool serialize(ISerializer& s);
	};

	void addGrain(IGrainData* grain, float in, float out, float easeIn, float easeOut);

	void removeGrain(IGrainData* grain);

	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

	const std::vector< GrainData >& getGrains() const { return m_grains; }

private:
	std::vector< GrainData > m_grains;
};

	}
}

#endif	// traktor_sound_EnvelopeGrainData_H
