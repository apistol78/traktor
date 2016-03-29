#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sound/IFilter.h"
#include "Sound/ISoundDriver.h"
#include "Sound/ISoundMixer.h"
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
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BlendGrain.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/EnvelopeGrain.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/InLoopOutGrain.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Resound/SimultaneousGrain.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Resound/TriggerGrain.h"
#include "Sound/Resound/TriggerGrainData.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundClassFactory", 0, SoundClassFactory, IRuntimeClassFactory)

void SoundClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Sound > > classSound = new AutoRuntimeClass< Sound >();
	classSound->addConstructor< ISoundBuffer*, handle_t, float, float, float, float >();
	classSound->addMethod("getBuffer", &Sound::getBuffer);
	classSound->addMethod("getCategory", &Sound::getCategory);
	classSound->addMethod("getVolume", &Sound::getVolume);
	classSound->addMethod("getPresence", &Sound::getPresence);
	classSound->addMethod("getPresenceRate", &Sound::getPresenceRate);
	classSound->addMethod("getRange", &Sound::getRange);
	registrar->registerClass(classSound);

	Ref< AutoRuntimeClass< SoundSystem > > classSoundSystem = new AutoRuntimeClass< SoundSystem >();
	classSoundSystem->addMethod("getChannel", &SoundSystem::getChannel);
	registrar->registerClass(classSoundSystem);

	Ref< AutoRuntimeClass< SoundChannel > > classSoundChannel = new AutoRuntimeClass< SoundChannel >();
	classSoundChannel->addMethod("setVolume", &SoundChannel::setVolume);
	classSoundChannel->addMethod("setFilter", &SoundChannel::setFilter);
	classSoundChannel->addMethod("isPlaying", &SoundChannel::isPlaying);
	classSoundChannel->addMethod("stop", &SoundChannel::stop);
	registrar->registerClass(classSoundChannel);

	Ref< AutoRuntimeClass< ISoundBuffer > > classISoundBuffer = new AutoRuntimeClass< ISoundBuffer >();
	registrar->registerClass(classISoundBuffer);

	Ref< AutoRuntimeClass< ISoundDriver > > classISoundDriver = new AutoRuntimeClass< ISoundDriver >();
	registrar->registerClass(classISoundDriver);

	Ref< AutoRuntimeClass< ISoundMixer > > classISoundMixer = new AutoRuntimeClass< ISoundMixer >();
	registrar->registerClass(classISoundMixer);

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

	Ref< AutoRuntimeClass< BankBuffer > > classBankBuffer = new AutoRuntimeClass< BankBuffer >();
	classBankBuffer->addConstructor< const RefArray< IGrain >& >();
	registrar->registerClass(classBankBuffer);

	Ref< AutoRuntimeClass< IGrain > > classIGrain = new AutoRuntimeClass< IGrain >();
	registrar->registerClass(classIGrain);

	Ref< AutoRuntimeClass< IGrainData > > classIGrainData = new AutoRuntimeClass< IGrainData >();
	registrar->registerClass(classIGrainData);

	Ref< AutoRuntimeClass< BlendGrain > > classBlendGrain = new AutoRuntimeClass< BlendGrain >();
	registrar->registerClass(classBlendGrain);

	Ref< AutoRuntimeClass< BlendGrainData > > classBlendGrainData = new AutoRuntimeClass< BlendGrainData >();
	classBlendGrainData->addConstructor();
	registrar->registerClass(classBlendGrainData);

	Ref< AutoRuntimeClass< EnvelopeGrain > > classEnvelopeGrain = new AutoRuntimeClass< EnvelopeGrain >();
	registrar->registerClass(classEnvelopeGrain);

	Ref< AutoRuntimeClass< EnvelopeGrainData > > classEnvelopeGrainData = new AutoRuntimeClass< EnvelopeGrainData >();
	classEnvelopeGrainData->addConstructor();
	registrar->registerClass(classEnvelopeGrainData);

	Ref< AutoRuntimeClass< InLoopOutGrain > > classInLoopOutGrain = new AutoRuntimeClass< InLoopOutGrain >();
	registrar->registerClass(classInLoopOutGrain);

	Ref< AutoRuntimeClass< InLoopOutGrainData > > classInLoopOutGrainData = new AutoRuntimeClass< InLoopOutGrainData >();
	classInLoopOutGrainData->addConstructor();
	registrar->registerClass(classInLoopOutGrainData);

	Ref< AutoRuntimeClass< MuteGrain > > classMuteGrain = new AutoRuntimeClass< MuteGrain >();
	registrar->registerClass(classMuteGrain);

	Ref< AutoRuntimeClass< MuteGrainData > > classMuteGrainData = new AutoRuntimeClass< MuteGrainData >();
	classMuteGrainData->addConstructor();
	registrar->registerClass(classMuteGrainData);

	Ref< AutoRuntimeClass< PlayGrain > > classPlayGrain = new AutoRuntimeClass< PlayGrain >();
	registrar->registerClass(classPlayGrain);

	Ref< AutoRuntimeClass< PlayGrainData > > classPlayGrainData = new AutoRuntimeClass< PlayGrainData >();
	classPlayGrainData->addConstructor();
	registrar->registerClass(classPlayGrainData);

	Ref< AutoRuntimeClass< RandomGrain > > classRandomGrain = new AutoRuntimeClass< RandomGrain >();
	registrar->registerClass(classRandomGrain);

	Ref< AutoRuntimeClass< RandomGrainData > > classRandomGrainData = new AutoRuntimeClass< RandomGrainData >();
	classRandomGrainData->addConstructor();
	registrar->registerClass(classRandomGrainData);

	Ref< AutoRuntimeClass< RepeatGrain > > classRepeatGrain = new AutoRuntimeClass< RepeatGrain >();
	registrar->registerClass(classRepeatGrain);

	Ref< AutoRuntimeClass< RepeatGrainData > > classRepeatGrainData = new AutoRuntimeClass< RepeatGrainData >();
	classRepeatGrainData->addConstructor();
	registrar->registerClass(classRepeatGrainData);

	Ref< AutoRuntimeClass< SequenceGrain > > classSequenceGrain = new AutoRuntimeClass< SequenceGrain >();
	registrar->registerClass(classSequenceGrain);

	Ref< AutoRuntimeClass< SequenceGrainData > > classSequenceGrainData = new AutoRuntimeClass< SequenceGrainData >();
	classSequenceGrainData->addConstructor();
	registrar->registerClass(classSequenceGrainData);

	Ref< AutoRuntimeClass< SimultaneousGrain > > classSimultaneousGrain = new AutoRuntimeClass< SimultaneousGrain >();
	registrar->registerClass(classSimultaneousGrain);

	Ref< AutoRuntimeClass< SimultaneousGrainData > > classSimultaneousGrainData = new AutoRuntimeClass< SimultaneousGrainData >();
	classSimultaneousGrainData->addConstructor();
	registrar->registerClass(classSimultaneousGrainData);

	Ref< AutoRuntimeClass< TriggerGrain > > classTriggerGrain = new AutoRuntimeClass< TriggerGrain >();
	registrar->registerClass(classTriggerGrain);

	Ref< AutoRuntimeClass< TriggerGrainData > > classTriggerGrainData = new AutoRuntimeClass< TriggerGrainData >();
	classTriggerGrainData->addConstructor();
	registrar->registerClass(classTriggerGrainData);
}

	}
}
