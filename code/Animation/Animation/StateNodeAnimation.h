/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	StateNodeAnimation(const std::wstring& name, const resource::IdProxy< Animation >& animation, bool linearInterpolation);

	virtual bool bind(resource::IResourceManager* resourceManager) override final;

	virtual bool prepareContext(StateContext& outContext) override final;

	virtual void evaluate(StateContext& context, Pose& outPose) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::IdProxy< Animation >& getAnimation() const { return m_animation; }

	bool getLinearInterpolation() const { return m_linearInterpolation; }

private:
	resource::IdProxy< Animation > m_animation;
	bool m_linearInterpolation;
};

	}
}

#endif	// traktor_animation_StateNodeAnimation_H
