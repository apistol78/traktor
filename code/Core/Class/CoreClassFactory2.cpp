/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/CoreClassFactory2.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/SHA1.h"
#include "Core/Timer/Timer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.CoreClassFactory2", 0, CoreClassFactory2, IRuntimeClassFactory)

void CoreClassFactory2::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Timer > > classTimer = new AutoRuntimeClass< Timer >();
	classTimer->addConstructor();
	classTimer->addMethod("start", &Timer::start);
	classTimer->addMethod("pause", &Timer::pause);
	classTimer->addMethod("stop", &Timer::stop);
	classTimer->addMethod("getElapsedTime", &Timer::getElapsedTime);
	classTimer->addMethod("getDeltaTime", &Timer::getDeltaTime);
	registrar->registerClass(classTimer);

	Ref< AutoRuntimeClass< Adler32 > > classAdler32 = new AutoRuntimeClass< Adler32 >();
	classAdler32->addConstructor();
	classAdler32->addMethod("begin", &Adler32::begin);
	classAdler32->addMethod("end", &Adler32::end);
	classAdler32->addMethod("get", &Adler32::get);
	registrar->registerClass(classAdler32);
	
	Ref< AutoRuntimeClass< Base64 > > classBase64 = new AutoRuntimeClass< Base64 >();
	registrar->registerClass(classBase64);

	Ref< AutoRuntimeClass< MD5 > > classMD5 = new AutoRuntimeClass< MD5 >();
	classMD5->addConstructor();
	classMD5->addMethod("create", &MD5::create);
	classMD5->addMethod("createFromString", &MD5::createFromString);
	classMD5->addMethod("begin", &MD5::begin);
	classMD5->addMethod("end", &MD5::end);
	classMD5->addMethod("format", &MD5::format);
	registrar->registerClass(classMD5);

	Ref< AutoRuntimeClass< SHA1 > > classSHA1 = new AutoRuntimeClass< SHA1 >();
	classSHA1->addConstructor();
	classSHA1->addMethod("createFromString", &SHA1::createFromString);
	classSHA1->addMethod("begin", &SHA1::begin);
	classSHA1->addMethod("end", &SHA1::end);
	classSHA1->addMethod("format", &SHA1::format);
	registrar->registerClass(classSHA1);
}

}
