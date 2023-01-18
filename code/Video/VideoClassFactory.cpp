/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Video/VideoClassFactory.h"
#include "Video/VideoTexture.h"

namespace traktor::video
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.video.VideoClassFactory", 0, VideoClassFactory, IRuntimeClassFactory)

void VideoClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classVideoTexture = new AutoRuntimeClass< VideoTexture >();
	registrar->registerClass(classVideoTexture);
}

}
