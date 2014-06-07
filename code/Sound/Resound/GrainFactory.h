#ifndef traktor_sound_GrainFactory_H
#define traktor_sound_GrainFactory_H

#include "Sound/Resound/IGrainFactory.h"

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

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS GrainFactory : public IGrainFactory
{
	T_RTTI_CLASS;

public:
	GrainFactory(resource::IResourceManager* resourceManager);

	virtual resource::IResourceManager* getResourceManager();

	virtual Ref< IGrain > createInstance(const IGrainData* grainData);

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_sound_GrainFactory_H
