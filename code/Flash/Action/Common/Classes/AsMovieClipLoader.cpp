/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/IFlashMovieLoader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Classes/AsMovieClipLoader.h"

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

void AsMovieClipLoader::initialize(ActionObject* self)
{
}

void AsMovieClipLoader::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsMovieClipLoader::xplicit(const ActionValueArray& args)
{
	return ActionValue();
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

bool AsMovieClipLoader::MovieClipLoader_loadClip(ActionObject* self, const std::wstring& url, FlashSpriteInstance* target) const
{
	ActionContext* cx = getContext();
	T_ASSERT (cx);

	const IFlashMovieLoader* movieLoader = cx->getMovieLoader();
	if (!movieLoader)
		return false;

	Ref< FlashMovie > movie = movieLoader->load(url);
	if (!movie)
		return false;

	return movie->createExternalMovieClipInstance(target, 0) != 0;
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
