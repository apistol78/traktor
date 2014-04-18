#ifndef traktor_world_PostProcessSettings_H
#define traktor_world_PostProcessSettings_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

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
class T_DLLCLASS PostProcessSettings : public ISerializable
{
	T_RTTI_CLASS;

public:
	PostProcessSettings();

	bool requireHighRange() const;

	const RefArray< PostProcessDefine >& getDefinitions() const;

	void setSteps(const RefArray< PostProcessStep >& steps);

	const RefArray< PostProcessStep >& getSteps() const;

	virtual void serialize(ISerializer& s);

private:
	bool m_requireHighRange;
	RefArray< PostProcessDefine > m_definitions;
	RefArray< PostProcessStep > m_steps;
};

	}
}

#endif	// traktor_world_PostProcessSettings_H
