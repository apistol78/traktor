#ifndef traktor_world_PostProcessSettings_H
#define traktor_world_PostProcessSettings_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class PostProcessDefine;
class PostProcessStep;

/*! \brief Post processing settings.
 * \ingroup World
 */
class T_DLLCLASS PostProcessSettings : public Serializable
{
	T_RTTI_CLASS(PostProcessSettings)

public:
	const RefArray< PostProcessDefine >& getDefinitions() const;

	const RefArray< PostProcessStep >& getSteps() const;

	virtual bool serialize(Serializer& s);

private:
	RefArray< PostProcessDefine > m_definitions;
	RefArray< PostProcessStep > m_steps;
};

	}
}

#endif	// traktor_world_PostProcessSettings_H
