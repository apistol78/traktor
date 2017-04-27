/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/RenderClassFactory.h"
#	include "Render/ImageProcess/ImageProcessDefineTarget.h"
#	include "Render/ImageProcess/ImageProcessDefineTexture.h"
#	include "Render/ImageProcess/ImageProcessStepBlur.h"
#	include "Render/ImageProcess/ImageProcessStepBokeh.h"
#	include "Render/ImageProcess/ImageProcessStepChain.h"
#	include "Render/ImageProcess/ImageProcessStepDiscardTarget.h"
#	include "Render/ImageProcess/ImageProcessStepGodRay.h"
#	include "Render/ImageProcess/ImageProcessStepGrain.h"
#	include "Render/ImageProcess/ImageProcessStepLensDirt.h"
#	include "Render/ImageProcess/ImageProcessStepLuminance.h"
#	include "Render/ImageProcess/ImageProcessStepRepeat.h"
#	include "Render/ImageProcess/ImageProcessStepSetTarget.h"
#	include "Render/ImageProcess/ImageProcessStepSimple.h"
#	include "Render/ImageProcess/ImageProcessStepSmProj.h"
#	include "Render/ImageProcess/ImageProcessStepSsao.h"
#	include "Render/ImageProcess/ImageProcessStepSwapTargets.h"
#	include "Render/ImageProcess/ImageProcessStepTemporal.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render()
{
	T_FORCE_LINK_REF(RenderClassFactory);
	T_FORCE_LINK_REF(ImageProcessDefineTarget);
	T_FORCE_LINK_REF(ImageProcessDefineTexture);
	T_FORCE_LINK_REF(ImageProcessStepBlur);
	T_FORCE_LINK_REF(ImageProcessStepBokeh);
	T_FORCE_LINK_REF(ImageProcessStepChain);
	T_FORCE_LINK_REF(ImageProcessStepDiscardTarget);
	T_FORCE_LINK_REF(ImageProcessStepGodRay);
	T_FORCE_LINK_REF(ImageProcessStepGrain);
	T_FORCE_LINK_REF(ImageProcessStepLensDirt);
	T_FORCE_LINK_REF(ImageProcessStepLuminance);
	T_FORCE_LINK_REF(ImageProcessStepRepeat);
	T_FORCE_LINK_REF(ImageProcessStepSetTarget);
	T_FORCE_LINK_REF(ImageProcessStepSimple);
	T_FORCE_LINK_REF(ImageProcessStepSmProj);
	T_FORCE_LINK_REF(ImageProcessStepSsao);
	T_FORCE_LINK_REF(ImageProcessStepSwapTargets);
	T_FORCE_LINK_REF(ImageProcessStepTemporal);
}

	}
}

#endif
