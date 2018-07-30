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
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/HashStream.h"
#include "Core/Misc/IHash.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/SHA1.h"
#include "Core/Timer/Timer.h"
#include "Core/Thread/Result.h"

namespace traktor
{
	namespace
	{
	
class ResultDeferred : public Result::IDeferred
{
public:
	ResultDeferred(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void dispatch(const Result& result) const
	{
		const Any argv[] =
		{
			Any::fromObject(const_cast< Result* >(&result))
		};
		m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

void Result_defer(Result* self, IRuntimeDelegate* delegate)
{
	self->defer(new ResultDeferred(delegate));
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.CoreClassFactory2", 0, CoreClassFactory2, IRuntimeClassFactory)

void CoreClassFactory2::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Timer > > classTimer = new AutoRuntimeClass< Timer >();
	classTimer->addConstructor();
	classTimer->addProperty("elapsedTime", &Timer::getElapsedTime);
	classTimer->addProperty("deltaTime", &Timer::getDeltaTime);
	classTimer->addMethod("start", &Timer::start);
	classTimer->addMethod("pause", &Timer::pause);
	classTimer->addMethod("stop", &Timer::stop);
	registrar->registerClass(classTimer);

	Ref< AutoRuntimeClass< IHash > > classIHash = new AutoRuntimeClass< IHash >();
	classIHash->addMethod("begin", &IHash::begin);
	classIHash->addMethod("end", &IHash::end);
	registrar->registerClass(classIHash);

	Ref< AutoRuntimeClass< Adler32 > > classAdler32 = new AutoRuntimeClass< Adler32 >();
	classAdler32->addConstructor();
	classAdler32->addMethod("get", &Adler32::get);
	registrar->registerClass(classAdler32);
	
	Ref< AutoRuntimeClass< Base64 > > classBase64 = new AutoRuntimeClass< Base64 >();
	registrar->registerClass(classBase64);

	Ref< AutoRuntimeClass< MD5 > > classMD5 = new AutoRuntimeClass< MD5 >();
	classMD5->addConstructor();
	classMD5->addMethod("create", &MD5::create);
	classMD5->addMethod("createFromString", &MD5::createFromString);
	classMD5->addMethod("format", &MD5::format);
	registrar->registerClass(classMD5);

	Ref< AutoRuntimeClass< SHA1 > > classSHA1 = new AutoRuntimeClass< SHA1 >();
	classSHA1->addConstructor();
	classSHA1->addMethod("createFromString", &SHA1::createFromString);
	classSHA1->addMethod("format", &SHA1::format);
	registrar->registerClass(classSHA1);

	Ref< AutoRuntimeClass< HashStream > > classHashStream = new AutoRuntimeClass< HashStream >();
	classHashStream->addConstructor< IHash* >();
	registrar->registerClass(classHashStream);

	Ref< AutoRuntimeClass< Result > > classResult = new AutoRuntimeClass< Result >();
	classResult->addProperty("ready", &Result::ready);
	classResult->addProperty("succeeded", &Result::succeeded);
	classResult->addMethod("succeed", &Result::succeed);
	classResult->addMethod("fail", &Result::fail);
	classResult->addMethod("defer", &Result_defer);
	classResult->addMethod("wait", &Result::succeeded);	// Mapping wait to succeeded, will block until result is available.
	registrar->registerClass(classResult);
}

}
