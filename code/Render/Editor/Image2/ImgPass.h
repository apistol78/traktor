#pragma once

#include "Core/RefArray.h"
#include "Render/Editor/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IImgStep;

/*! Image pass definition
 * \ingroup Render
 *
 * A pass contain multiple steps which describe how this pass
 * should be rendered.
 */
class T_DLLCLASS ImgPass : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgPass();

	const RefArray< IImgStep >& getSteps() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< IImgStep > m_steps;
};

	}
}