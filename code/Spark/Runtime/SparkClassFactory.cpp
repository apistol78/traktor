#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Spark/Movie.h"
#include "Spark/MoviePlayer.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Action/ActionContext.h"
#include "Spark/Runtime/SparkClassFactory.h"
#include "Spark/Runtime/SparkLayer.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

Any SparkLayer_externalCall(SparkLayer* self, const std::string& methodName, uint32_t argc, const Any* argv)
{
	return self->externalCall(methodName, argc, argv);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkClassFactory", 0, SparkClassFactory, IRuntimeClassFactory)

void SparkClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classSparkLayer = new AutoRuntimeClass< SparkLayer >();
	classSparkLayer->addProperty< MoviePlayer* >("moviePlayer", 0, &SparkLayer::getMoviePlayer);
	classSparkLayer->addProperty< ActionContext* >("context", 0, &SparkLayer::getContext);
	classSparkLayer->addProperty< SpriteInstance* >("root", 0, &SparkLayer::getRoot);
	classSparkLayer->addProperty< bool >("visible", &SparkLayer::setVisible, &SparkLayer::isVisible);
	classSparkLayer->addMethod("getExternal", &SparkLayer::getExternal);
	classSparkLayer->addMethod("getPrintableString", &SparkLayer::getPrintableString);
	classSparkLayer->setUnknownHandler(&SparkLayer_externalCall);
	registrar->registerClass(classSparkLayer);
}

	}
}