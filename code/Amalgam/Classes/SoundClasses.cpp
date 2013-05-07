#include "Amalgam/Classes/SoundClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"
#include "Sound/IFilter.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/CombFilter.h"
#include "Sound/Filters/DitherFilter.h"
#include "Sound/Filters/EqualizerFilter.h"
#include "Sound/Filters/FFTFilter.h"
#include "Sound/Filters/GroupFilter.h"
#include "Sound/Filters/LowPassFilter.h"
#include "Sound/Filters/NormalizationFilter.h"
#include "Sound/Filters/RingModulationFilter.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"

namespace traktor
{
	namespace amalgam
	{

void registerSoundClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< sound::SoundSystem > > classSoundSystem = new script::AutoScriptClass< sound::SoundSystem >();
	classSoundSystem->addMethod("getChannel", &sound::SoundSystem::getChannel);
	scriptManager->registerClass(classSoundSystem);

	Ref< script::AutoScriptClass< sound::SoundChannel > > classSoundChannel = new script::AutoScriptClass< sound::SoundChannel >();
	classSoundChannel->addMethod("setVolume", &sound::SoundChannel::setVolume);
	classSoundChannel->addMethod("setFilter", &sound::SoundChannel::setFilter);
	classSoundChannel->addMethod("getFilter", &sound::SoundChannel::getFilter);
	classSoundChannel->addMethod("isPlaying", &sound::SoundChannel::isPlaying);
	classSoundChannel->addMethod("stop", &sound::SoundChannel::stop);
	scriptManager->registerClass(classSoundChannel);

	Ref< script::AutoScriptClass< sound::IFilter > > classIFilter = new script::AutoScriptClass< sound::IFilter >();
	scriptManager->registerClass(classIFilter);

	Ref< script::AutoScriptClass< sound::SurroundEnvironment > > classSurroundEnvironment = new script::AutoScriptClass< sound::SurroundEnvironment >();
	classSurroundEnvironment->addMethod("setMaxDistance", &sound::SurroundEnvironment::setMaxDistance);
	classSurroundEnvironment->addMethod("getMaxDistance", &sound::SurroundEnvironment::getMaxDistance);
	classSurroundEnvironment->addMethod("setInnerRadius", &sound::SurroundEnvironment::setInnerRadius);
	classSurroundEnvironment->addMethod("getInnerRadius", &sound::SurroundEnvironment::getInnerRadius);
	classSurroundEnvironment->addMethod("setFullSurround", &sound::SurroundEnvironment::setFullSurround);
	classSurroundEnvironment->addMethod("getFullSurround", &sound::SurroundEnvironment::getFullSurround);
	classSurroundEnvironment->addMethod("setListenerTransform", &sound::SurroundEnvironment::setListenerTransform);
	classSurroundEnvironment->addMethod("getListenerTransform", &sound::SurroundEnvironment::getListenerTransform);
	classSurroundEnvironment->addMethod("getListenerTransformInv", &sound::SurroundEnvironment::getListenerTransformInv);
	scriptManager->registerClass(classSurroundEnvironment);

	Ref< script::AutoScriptClass< sound::CombFilter > > classCombFilter = new script::AutoScriptClass< sound::CombFilter >();
	classCombFilter->addConstructor();
	classCombFilter->addConstructor< uint32_t >();
	classCombFilter->addConstructor< uint32_t, float >();
	classCombFilter->addConstructor< uint32_t, float, float >();
	scriptManager->registerClass(classCombFilter);

	Ref< script::AutoScriptClass< sound::DitherFilter > > classDitherFilter = new script::AutoScriptClass< sound::DitherFilter >();
	classDitherFilter->addConstructor();
	classDitherFilter->addConstructor< uint32_t >();
	scriptManager->registerClass(classDitherFilter);

	Ref< script::AutoScriptClass< sound::EqualizerFilter > > classEqualizerFilter = new script::AutoScriptClass< sound::EqualizerFilter >();
	classEqualizerFilter->addConstructor();
	classEqualizerFilter->addConstructor< float >();
	scriptManager->registerClass(classEqualizerFilter);

	Ref< script::AutoScriptClass< sound::FFTFilter > > classFFTFilter = new script::AutoScriptClass< sound::FFTFilter >();
	classFFTFilter->addConstructor();
	classFFTFilter->addConstructor< uint32_t >();
	scriptManager->registerClass(classFFTFilter);

	Ref< script::AutoScriptClass< sound::GroupFilter > > classGroupFilter = new script::AutoScriptClass< sound::GroupFilter >();
	classGroupFilter->addConstructor();
	classGroupFilter->addMethod("addFilter", &sound::GroupFilter::addFilter);
	scriptManager->registerClass(classGroupFilter);

	Ref< script::AutoScriptClass< sound::LowPassFilter > > classLowPassFilter = new script::AutoScriptClass< sound::LowPassFilter >();
	classLowPassFilter->addConstructor();
	classLowPassFilter->addConstructor< float >();
	classLowPassFilter->addMethod("setCutOff", &sound::LowPassFilter::setCutOff);
	classLowPassFilter->addMethod("getCutOff", &sound::LowPassFilter::getCutOff);
	scriptManager->registerClass(classLowPassFilter);

	Ref< script::AutoScriptClass< sound::NormalizationFilter > > classNormalizationFilter = new script::AutoScriptClass< sound::NormalizationFilter >();
	classNormalizationFilter->addConstructor();
	classNormalizationFilter->addConstructor< float >();
	classNormalizationFilter->addConstructor< float, float >();
	classNormalizationFilter->addConstructor< float, float, float >();
	classNormalizationFilter->addMethod("getTargetEnergy", &sound::NormalizationFilter::getTargetEnergy);
	classNormalizationFilter->addMethod("getEnergyThreshold", &sound::NormalizationFilter::getEnergyThreshold);
	classNormalizationFilter->addMethod("getAttackRate", &sound::NormalizationFilter::getAttackRate);
	scriptManager->registerClass(classNormalizationFilter);

	Ref< script::AutoScriptClass< sound::RingModulationFilter > > classRingModulationFilter = new script::AutoScriptClass< sound::RingModulationFilter >();
	classRingModulationFilter->addConstructor();
	classRingModulationFilter->addConstructor< uint32_t >();
	classRingModulationFilter->addConstructor< uint32_t, uint32_t >();
	scriptManager->registerClass(classRingModulationFilter);

	Ref< script::AutoScriptClass< sound::SurroundFilter > > classSurroundFilter = new script::AutoScriptClass< sound::SurroundFilter >();
	classSurroundFilter->addConstructor< sound::SurroundEnvironment* >();
	classSurroundFilter->addConstructor< sound::SurroundEnvironment*, const Vector4& >();
	classSurroundFilter->addMethod("setSpeakerPosition", &sound::SurroundFilter::setSpeakerPosition);
	scriptManager->registerClass(classSurroundFilter);
}

	}
}
