/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
