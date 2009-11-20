#include "Flash/Action/Classes/AsMovieClipLoader.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMovieClipLoader", AsMovieClipLoader, ActionClass)

Ref< AsMovieClipLoader > AsMovieClipLoader::getInstance()
{
	static Ref< AsMovieClipLoader > instance = 0;
	if (!instance)
	{
		instance = new AsMovieClipLoader();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsMovieClipLoader::AsMovieClipLoader()
:	ActionClass(L"MovieClipLoader")
{
}

void AsMovieClipLoader::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"addListener", createNativeFunctionValue(this, &AsMovieClipLoader::MovieClipLoader_addListener));
	prototype->setMember(L"getProgress", createNativeFunctionValue(this, &AsMovieClipLoader::MovieClipLoader_getProgress));
	prototype->setMember(L"loadClip", createNativeFunctionValue(this, &AsMovieClipLoader::MovieClipLoader_loadClip));
	prototype->setMember(L"removeListener", createNativeFunctionValue(this, &AsMovieClipLoader::MovieClipLoader_removeListener));
	prototype->setMember(L"unloadClip", createNativeFunctionValue(this, &AsMovieClipLoader::MovieClipLoader_unloadClip));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsMovieClipLoader::construct(ActionContext* context, const args_t& args)
{
	return ActionValue::fromObject(new ActionObject(this));
}

void AsMovieClipLoader::MovieClipLoader_addListener(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.addListener not implemented" << Endl;
	ca.ret = ActionValue(false);
}

void AsMovieClipLoader::MovieClipLoader_getProgress(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.getProgress not implemented" << Endl;
	ca.ret = ActionValue(0.0);
}

void AsMovieClipLoader::MovieClipLoader_loadClip(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.loadClip not implemented" << Endl;
	ca.ret = ActionValue(false);
}

void AsMovieClipLoader::MovieClipLoader_removeListener(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.removeListener not implemented" << Endl;
	ca.ret = ActionValue(false);
}

void AsMovieClipLoader::MovieClipLoader_unloadClip(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.unloadClip not implemented" << Endl;
	ca.ret = ActionValue(false);
}

	}
}
