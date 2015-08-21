#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Telemetry/TelemetryClassFactory.h"
#include "Telemetry/TelemetryClient.h"

namespace traktor
{
	namespace telemetry
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.telemetry.TelemetryClassFactory", 0, TelemetryClassFactory, IRuntimeClassFactory)

void TelemetryClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< TelemetryClient > > classTelemetryClient = new AutoRuntimeClass< TelemetryClient >();
	classTelemetryClient->addStaticMethod("create", &TelemetryClient::create);
	classTelemetryClient->addMethod("destroy", &TelemetryClient::destroy);
	classTelemetryClient->addMethod("set", &TelemetryClient::set);
	classTelemetryClient->addMethod("add", &TelemetryClient::add);
	classTelemetryClient->addMethod("flush", &TelemetryClient::flush);
	registrar->registerClass(classTelemetryClient);
}

	}
}
