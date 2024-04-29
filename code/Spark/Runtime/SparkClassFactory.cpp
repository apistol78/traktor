/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Spark/Context.h"
#include "Spark/Movie.h"
#include "Spark/MoviePlayer.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Runtime/SparkClassFactory.h"
#include "Spark/Runtime/SparkLayer.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkClassFactory", 0, SparkClassFactory, IRuntimeClassFactory)

void SparkClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classSparkLayer = new AutoRuntimeClass< SparkLayer >();
	classSparkLayer->addProperty< MoviePlayer* >("moviePlayer", 0, &SparkLayer::getMoviePlayer);
	classSparkLayer->addProperty< Context* >("context", 0, &SparkLayer::getContext);
	classSparkLayer->addProperty< SpriteInstance* >("root", 0, &SparkLayer::getRoot);
	classSparkLayer->addProperty< bool >("visible", &SparkLayer::setVisible, &SparkLayer::isVisible);
	classSparkLayer->addMethod("getExternal", &SparkLayer::getExternal);
	classSparkLayer->addMethod("getPrintableString", &SparkLayer::getPrintableString);
	registrar->registerClass(classSparkLayer);
}

}
