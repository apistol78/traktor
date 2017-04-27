/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Animation/AnimationClassFactory.h"
#	include "Animation/Animation/SimpleAnimationControllerData.h"
#	include "Animation/Animation/StatePoseControllerData.h"
#	include "Animation/Animation/StateNodeAnimation.h"
#	include "Animation/IK/IKPoseControllerData.h"
#	include "Animation/RagDoll/RagDollPoseControllerData.h"

namespace traktor
{
	namespace animation
	{

extern "C" void __module__Traktor_Animation()
{
	T_FORCE_LINK_REF(AnimationClassFactory);
	T_FORCE_LINK_REF(SimpleAnimationControllerData);
	T_FORCE_LINK_REF(StatePoseControllerData);
	T_FORCE_LINK_REF(StateNodeAnimation);
	T_FORCE_LINK_REF(IKPoseControllerData);
	T_FORCE_LINK_REF(RagDollPoseControllerData);
}

	}
}

#endif
