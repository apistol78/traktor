#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sound/IFilter.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundClassFactory.h"
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
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundClassFactory", 0, SoundClassFactory, IRuntimeClassFactory)

void SoundClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< SoundSystem > > classSoundSystem = new AutoRuntimeClass< SoundSystem >();
	classSoundSystem->addMethod("getChannel", &SoundSystem::getChannel);
	registrar->registerClass(classSoundSystem);

	Ref< AutoRuntimeClass< SoundChannel > > classSoundChannel = new AutoRuntimeClass< SoundChannel >();
	classSoundChannel->addMethod("setVolume", &SoundChannel::setVolume);
	classSoundChannel->addMethod("setFilter", &SoundChannel::setFilter);
	classSoundChannel->addMethod("isPlaying", &SoundChannel::isPlaying);
	classSoundChannel->addMethod("stop", &SoundChannel::stop);
	registrar->registerClass(classSoundChannel);

	Ref< AutoRuntimeClass< IFilter > > classIFilter = new AutoRuntimeClass< IFilter >();
	registrar->registerClass(classIFilter);

	Ref< AutoRuntimeClass< SurroundEnvironment > > classSurroundEnvironment = new AutoRuntimeClass< SurroundEnvironment >();
	classSurroundEnvironment->addMethod("setMaxDistance", &SurroundEnvironment::setMaxDistance);
	classSurroundEnvironment->addMethod("getMaxDistance", &SurroundEnvironment::getMaxDistance);
	classSurroundEnvironment->addMethod("setInnerRadius", &SurroundEnvironment::setInnerRadius);
	classSurroundEnvironment->addMethod("getInnerRadius", &SurroundEnvironment::getInnerRadius);
	classSurroundEnvironment->addMethod("setFullSurround", &SurroundEnvironment::setFullSurround);
	classSurroundEnvironment->addMethod("getFullSurround", &SurroundEnvironment::getFullSurround);
	classSurroundEnvironment->addMethod("setListenerTransform", &SurroundEnvironment::setListenerTransform);
	classSurroundEnvironment->addMethod("getListenerTransform", &SurroundEnvironment::getListenerTransform);
	classSurroundEnvironment->addMethod("getListenerTransformInv", &SurroundEnvironment::getListenerTransformInv);
	registrar->registerClass(classSurroundEnvironment);

	Ref< AutoRuntimeClass< CombFilter > > classCombFilter = new AutoRuntimeClass< CombFilter >();
	classCombFilter->addConstructor();
	classCombFilter->addConstructor< uint32_t >();
	classCombFilter->addConstructor< uint32_t, float >();
	classCombFilter->addConstructor< uint32_t, float, float >();
	registrar->registerClass(classCombFilter);

	Ref< AutoRuntimeClass< DitherFilter > > classDitherFilter = new AutoRuntimeClass< DitherFilter >();
	classDitherFilter->addConstructor();
	classDitherFilter->addConstructor< uint32_t >();
	registrar->registerClass(classDitherFilter);

	Ref< AutoRuntimeClass< EqualizerFilter > > classEqualizerFilter = new AutoRuntimeClass< EqualizerFilter >();
	classEqualizerFilter->addConstructor();
	classEqualizerFilter->addConstructor< float >();
	registrar->registerClass(classEqualizerFilter);

	Ref< AutoRuntimeClass< FFTFilter > > classFFTFilter = new AutoRuntimeClass< FFTFilter >();
	classFFTFilter->addConstructor();
	classFFTFilter->addConstructor< uint32_t >();
	registrar->registerClass(classFFTFilter);

	Ref< AutoRuntimeClass< GroupFilter > > classGroupFilter = new AutoRuntimeClass< GroupFilter >();
	classGroupFilter->addConstructor();
	classGroupFilter->addMethod("addFilter", &GroupFilter::addFilter);
	registrar->registerClass(classGroupFilter);

	Ref< AutoRuntimeClass< LowPassFilter > > classLowPassFilter = new AutoRuntimeClass< LowPassFilter >();
	classLowPassFilter->addConstructor();
	classLowPassFilter->addConstructor< float >();
	classLowPassFilter->addMethod("setCutOff", &LowPassFilter::setCutOff);
	classLowPassFilter->addMethod("getCutOff", &LowPassFilter::getCutOff);
	registrar->registerClass(classLowPassFilter);

	Ref< AutoRuntimeClass< NormalizationFilter > > classNormalizationFilter = new AutoRuntimeClass< NormalizationFilter >();
	classNormalizationFilter->addConstructor();
	classNormalizationFilter->addConstructor< float >();
	classNormalizationFilter->addConstructor< float, float >();
	classNormalizationFilter->addConstructor< float, float, float >();
	classNormalizationFilter->addMethod("getTargetEnergy", &NormalizationFilter::getTargetEnergy);
	classNormalizationFilter->addMethod("getEnergyThreshold", &NormalizationFilter::getEnergyThreshold);
	classNormalizationFilter->addMethod("getAttackRate", &NormalizationFilter::getAttackRate);
	registrar->registerClass(classNormalizationFilter);

	Ref< AutoRuntimeClass< RingModulationFilter > > classRingModulationFilter = new AutoRuntimeClass< RingModulationFilter >();
	classRingModulationFilter->addConstructor();
	classRingModulationFilter->addConstructor< uint32_t >();
	classRingModulationFilter->addConstructor< uint32_t, uint32_t >();
	registrar->registerClass(classRingModulationFilter);

	Ref< AutoRuntimeClass< SurroundFilter > > classSurroundFilter = new AutoRuntimeClass< SurroundFilter >();
	classSurroundFilter->addConstructor< SurroundEnvironment* >();
	classSurroundFilter->addConstructor< SurroundEnvironment*, const Vector4& >();
	classSurroundFilter->addMethod("setSpeakerPosition", &SurroundFilter::setSpeakerPosition);
	registrar->registerClass(classSurroundFilter);

	Ref< AutoRuntimeClass< ISoundHandle > > classISoundHandle = new AutoRuntimeClass< ISoundHandle >();
	classISoundHandle->addMethod("stop", &ISoundHandle::stop);
	classISoundHandle->addMethod("fadeOff", &ISoundHandle::fadeOff);
	classISoundHandle->addMethod("isPlaying", &ISoundHandle::isPlaying);
	classISoundHandle->addMethod("setVolume", &ISoundHandle::setVolume);
	classISoundHandle->addMethod("setPitch", &ISoundHandle::setPitch);
	classISoundHandle->addMethod("setPosition", &ISoundHandle::setPosition);
	classISoundHandle->addMethod("setParameter", &ISoundHandle::setParameter);
	registrar->registerClass(classISoundHandle);

	Ref< AutoRuntimeClass< ISoundPlayer > > classISoundPlayer = new AutoRuntimeClass< ISoundPlayer >();
	classISoundPlayer->addMethod("play", &ISoundPlayer::play);
	classISoundPlayer->addMethod("play3d", &ISoundPlayer::play3d);
	classISoundPlayer->addMethod("setListenerTransform", &ISoundPlayer::setListenerTransform);
	classISoundPlayer->addMethod("getListenerTransform", &ISoundPlayer::getListenerTransform);
	registrar->registerClass(classISoundPlayer);
}

	}
}
