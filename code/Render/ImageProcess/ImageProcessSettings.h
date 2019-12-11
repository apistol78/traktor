#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageProcessDefine;
class ImageProcessStep;

/*! Post processing settings.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessSettings : public ISerializable
{
	T_RTTI_CLASS;

public:
	ImageProcessSettings();

	bool requireHighRange() const;

	void setDefinitions(const RefArray< ImageProcessDefine >& definitions);

	const RefArray< ImageProcessDefine >& getDefinitions() const;

	void setSteps(const RefArray< ImageProcessStep >& steps);

	const RefArray< ImageProcessStep >& getSteps() const;

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_requireHighRange;
	RefArray< ImageProcessDefine > m_definitions;
	RefArray< ImageProcessStep > m_steps;
};

	}
}

