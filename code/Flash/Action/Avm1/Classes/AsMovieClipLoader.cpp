#include "Core/Log/Log.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsMovieClipLoader.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsMovieClipLoader", AsMovieClipLoader, ActionClass)

AsMovieClipLoader::AsMovieClipLoader(ActionContext* context)
:	ActionClass(context, "MovieClipLoader")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("addListener", ActionValue(createNativeFunction(context, this, &AsMovieClipLoader::MovieClipLoader_addListener)));
	prototype->setMember("getProgress", ActionValue(createNativeFunction(context, this, &AsMovieClipLoader::MovieClipLoader_getProgress)));
	prototype->setMember("loadClip", ActionValue(createNativeFunction(context, this, &AsMovieClipLoader::MovieClipLoader_loadClip)));
	prototype->setMember("removeListener", ActionValue(createNativeFunction(context, this, &AsMovieClipLoader::MovieClipLoader_removeListener)));
	prototype->setMember("unloadClip", ActionValue(createNativeFunction(context, this, &AsMovieClipLoader::MovieClipLoader_unloadClip)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsMovieClipLoader::init(ActionObject* self, const ActionValueArray& args)
{
}

void AsMovieClipLoader::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsMovieClipLoader::MovieClipLoader_addListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"MovieClipLoader::addListener not implemented" << Endl;
	)
}

void AsMovieClipLoader::MovieClipLoader_getProgress(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"MovieClipLoader::getProgress not implemented" << Endl;
	)
}

void AsMovieClipLoader::MovieClipLoader_loadClip(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"MovieClipLoader::loadClip not implemented" << Endl;
	)
}

void AsMovieClipLoader::MovieClipLoader_removeListener(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"MovieClipLoader::removeListener not implemented" << Endl;
	)
}

void AsMovieClipLoader::MovieClipLoader_unloadClip(CallArgs& ca)
{
	T_IF_VERBOSE(
		log::warning << L"MovieClipLoader::unloadClip not implemented" << Endl;
	)
}

	}
}
