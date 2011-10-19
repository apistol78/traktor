#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMovieClipLoader.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMovieClipLoader", AsMovieClipLoader, ActionClass)

AsMovieClipLoader::AsMovieClipLoader()
:	ActionClass("MovieClipLoader")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("addListener", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_addListener)));
	prototype->setMember("getProgress", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_getProgress)));
	prototype->setMember("loadClip", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_loadClip)));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_removeListener)));
	prototype->setMember("unloadClip", ActionValue(createNativeFunction(this, &AsMovieClipLoader::MovieClipLoader_unloadClip)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsMovieClipLoader::alloc(ActionContext* context)
{
	return new ActionObject("MovieClipLoader");
}

void AsMovieClipLoader::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
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
