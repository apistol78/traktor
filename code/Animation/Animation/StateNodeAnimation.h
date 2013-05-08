#ifndef traktor_animation_StateNodeAnimation_H
#define traktor_animation_StateNodeAnimation_H

#include "Animation/Animation/StateNode.h"
#include "Resource/IdProxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class Animation;

/*! \brief Animation state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeAnimation : public StateNode
{
	T_RTTI_CLASS;

public:
	StateNodeAnimation();

	StateNodeAnimation(const std::wstring& name, const resource::IdProxy< Animation >& animation);

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual bool prepareContext(StateContext& outContext);

	virtual void evaluate(const StateContext& context, Pose& outPose);

	virtual void serialize(ISerializer& s);

	const resource::IdProxy< Animation >& getAnimation() const { return m_animation; }

private:
	resource::IdProxy< Animation > m_animation;
};

	}
}

#endif	// traktor_animation_StateNodeAnimation_H
