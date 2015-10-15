#ifndef traktor_spark_Context_H
#define traktor_spark_Context_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS Context : public Object
{
	T_RTTI_CLASS;

public:
	Context(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::ISoundPlayer* soundPlayer
	);

	resource::IResourceManager* getResourceManager() const;

	render::IRenderSystem* getRenderSystem() const;

	sound::ISoundPlayer* getSoundPlayer() const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< sound::ISoundPlayer > m_soundPlayer;
};

	}
}

#endif	// traktor_spark_Context_H
