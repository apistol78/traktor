/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Animation/AnimationClassFactory.h"
#	include "Animation/RagDoll/RagDollPoseControllerData.h"
#	include "Animation/Simple/SimpleAnimationControllerData.h"
#	include "Animation/Stack/StackAnimationControllerData.h"
#	include "Animation/StateGraph/AnimationGraphPoseControllerData.h"
#	include "Animation/StateGraph/RtStateData.h"
#	include "Animation/StateGraph/RtStateGraphData.h"
#	include "Animation/StateGraph/RtStateTransitionData.h"

namespace traktor::animation
{

extern "C" void __module__Traktor_Animation()
{
	T_FORCE_LINK_REF(AnimationClassFactory);
	T_FORCE_LINK_REF(AnimationGraphPoseControllerData);
	T_FORCE_LINK_REF(RagDollPoseControllerData);
	T_FORCE_LINK_REF(SimpleAnimationControllerData);
	T_FORCE_LINK_REF(StackAnimationControllerData);
	T_FORCE_LINK_REF(RtStateData);
	T_FORCE_LINK_REF(RtStateGraphData);
	T_FORCE_LINK_REF(RtStateTransitionData);
}

}

#endif
