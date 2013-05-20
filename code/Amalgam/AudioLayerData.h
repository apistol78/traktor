#ifndef traktor_amalgam_AudioLayerData_H
#define traktor_amalgam_AudioLayerData_H

#include "Amalgam/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

	}

	namespace amalgam
	{

class T_DLLCLASS AudioLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	AudioLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< sound::Sound > m_sound;
	bool m_autoPlay;
	bool m_repeat;
};

	}
}

#endif	// traktor_amalgam_AudioLayerData_H
