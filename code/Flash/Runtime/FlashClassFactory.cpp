#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Flash/Movie.h"
#include "Flash/MoviePlayer.h"
#include "Flash/SpriteInstance.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Runtime/FlashClassFactory.h"
#include "Flash/Runtime/FlashLayer.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

Any FlashLayer_externalCall(FlashLayer* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return self->externalCall(methodName, argc, argv);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashClassFactory", 0, FlashClassFactory, IRuntimeClassFactory)

void FlashClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classFlashLayer = new AutoRuntimeClass< FlashLayer >();
	classFlashLayer->addProperty< flash::MoviePlayer* >("moviePlayer", 0, &FlashLayer::getMoviePlayer);
	classFlashLayer->addProperty< flash::ActionContext* >("context", 0, &FlashLayer::getContext);
	classFlashLayer->addProperty< flash::SpriteInstance* >("root", 0, &FlashLayer::getRoot);
	classFlashLayer->addProperty< bool >("visible", &FlashLayer::setVisible, &FlashLayer::isVisible);
	classFlashLayer->addMethod("getExternal", &FlashLayer::getExternal);
	classFlashLayer->addMethod("getPrintableString", &FlashLayer::getPrintableString);
	classFlashLayer->setUnknownHandler(&FlashLayer_externalCall);
	registrar->registerClass(classFlashLayer);
}

	}
}