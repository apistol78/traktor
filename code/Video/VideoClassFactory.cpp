/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Video/VideoClassFactory.h"
#include "Video/VideoTexture.h"

namespace traktor
{
	namespace video
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoClassFactory", 0, VideoClassFactory, IRuntimeClassFactory)

void VideoClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< VideoTexture > > classVideoTexture = new AutoRuntimeClass< VideoTexture >();
	registrar->registerClass(classVideoTexture);
}

	}
}
