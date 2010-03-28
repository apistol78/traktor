#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMovieClipLoader.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

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

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"addListener", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_addListener)));
	prototype->setMember(L"getProgress", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_getProgress)));
	prototype->setMember(L"loadClip", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_loadClip)));
	prototype->setMember(L"removeListener", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_removeListener)));
	prototype->setMember(L"unloadClip", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_unloadClip)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsMovieClipLoader::construct(ActionContext* context, const args_t& args)
{
	return ActionValue(new ActionObject(this));
}

void AsMovieClipLoader::MovieClipLoader_addListener(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.addListener not implemented" << Endl;
	ca.ret = ActionValue(false);
}

void AsMovieClipLoader::MovieClipLoader_getProgress(CallArgs& ca)
{
	log::warning << L"MovieClipLoader.getProgress not implemented" << Endl;
	ca.ret = ActionValue(avm_number_t(0));
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
