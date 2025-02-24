/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sound/AudioChannel.h"
#include "Sound/AudioClassFactory.h"
#include "Sound/AudioSystem.h"
#include "Sound/BoxedAudioBlock.h"
#include "Sound/IAudioDriver.h"
#include "Sound/IAudioFilter.h"
#include "Sound/IAudioMixer.h"
#include "Sound/Sound.h"
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
#include "Sound/Player/SoundHandle.h"
#include "Sound/Player/SoundPlayer.h"
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

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.AudioClassFactory", 0, AudioClassFactory, IRuntimeClassFactory)

void AudioClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classAudioBlock = new AutoRuntimeClass< BoxedAudioBlock >();
	classAudioBlock->addProperty("samplesCount", &BoxedAudioBlock::getSamplesCount);
	classAudioBlock->addProperty("sampleRate", &BoxedAudioBlock::getSampleRate);
	classAudioBlock->addProperty("maxChannel", &BoxedAudioBlock::getMaxChannel);
	classAudioBlock->addMethod("get", &BoxedAudioBlock::get);
	classAudioBlock->addMethod("set", &BoxedAudioBlock::set);
	registrar->registerClass(classAudioBlock);

	auto classSound = new AutoRuntimeClass< Sound >();
	classSound->addConstructor< IAudioBuffer*, handle_t, float, float >();
	classSound->addProperty("buffer", &Sound::getBuffer);
	classSound->addProperty("category", &Sound::getCategory);
	classSound->addProperty("gain", &Sound::getGain);
	classSound->addProperty("range", &Sound::getRange);
	registrar->registerClass(classSound);

	auto classSoundSystem = new AutoRuntimeClass< AudioSystem >();
	classSoundSystem->addConstructor< IAudioDriver* >();
	classSoundSystem->addMethod("getChannel", &AudioSystem::getChannel);
	registrar->registerClass(classSoundSystem);

	auto classAudioChannel = new AutoRuntimeClass< AudioChannel >();
	classAudioChannel->addProperty("volume", &AudioChannel::setVolume, &AudioChannel::getVolume);
	classAudioChannel->addProperty("pitch", &AudioChannel::setPitch, &AudioChannel::getPitch);
	classAudioChannel->addProperty("playing", &AudioChannel::isPlaying);
	classAudioChannel->addMethod("setFilter", &AudioChannel::setFilter);
	classAudioChannel->addMethod("stop", &AudioChannel::stop);
	registrar->registerClass(classAudioChannel);

	auto classIAudioBuffer = new AutoRuntimeClass< IAudioBuffer >();
	registrar->registerClass(classIAudioBuffer);

	auto classIAudioDriver = new AutoRuntimeClass< IAudioDriver >();
	registrar->registerClass(classIAudioDriver);

	auto classIAudioMixer = new AutoRuntimeClass< IAudioMixer >();
	registrar->registerClass(classIAudioMixer);

	auto classIAudioFilter = new AutoRuntimeClass< IAudioFilter >();
	registrar->registerClass(classIAudioFilter);

	auto classSurroundEnvironment = new AutoRuntimeClass< SurroundEnvironment >();
	classSurroundEnvironment->addProperty("maxDistance", &SurroundEnvironment::setMaxDistance, &SurroundEnvironment::getMaxDistance);
	classSurroundEnvironment->addProperty("innerRadius", &SurroundEnvironment::setInnerRadius, &SurroundEnvironment::getInnerRadius);
	classSurroundEnvironment->addProperty("fullSurround", &SurroundEnvironment::setFullSurround, &SurroundEnvironment::getFullSurround);
	registrar->registerClass(classSurroundEnvironment);

	auto classCombFilter = new AutoRuntimeClass< CombFilter >();
	classCombFilter->addConstructor();
	classCombFilter->addConstructor< uint32_t >();
	classCombFilter->addConstructor< uint32_t, float >();
	classCombFilter->addConstructor< uint32_t, float, float >();
	registrar->registerClass(classCombFilter);

	auto classDitherFilter = new AutoRuntimeClass< DitherFilter >();
	classDitherFilter->addConstructor();
	classDitherFilter->addConstructor< uint32_t >();
	registrar->registerClass(classDitherFilter);

	auto classEqualizerFilter = new AutoRuntimeClass< EqualizerFilter >();
	classEqualizerFilter->addConstructor();
	classEqualizerFilter->addConstructor< float >();
	registrar->registerClass(classEqualizerFilter);

	auto classFFTFilter = new AutoRuntimeClass< FFTFilter >();
	classFFTFilter->addConstructor();
	classFFTFilter->addConstructor< uint32_t >();
	registrar->registerClass(classFFTFilter);

	auto classGroupFilter = new AutoRuntimeClass< GroupFilter >();
	classGroupFilter->addConstructor();
	classGroupFilter->addMethod("addFilter", &GroupFilter::addFilter);
	registrar->registerClass(classGroupFilter);

	auto classLowPassFilter = new AutoRuntimeClass< LowPassFilter >();
	classLowPassFilter->addConstructor();
	classLowPassFilter->addConstructor< float >();
	classLowPassFilter->addProperty("cutOff", &LowPassFilter::setCutOff, &LowPassFilter::getCutOff);
	registrar->registerClass(classLowPassFilter);

	auto classNormalizationFilter = new AutoRuntimeClass< NormalizationFilter >();
	classNormalizationFilter->addConstructor();
	classNormalizationFilter->addConstructor< float >();
	classNormalizationFilter->addConstructor< float, float >();
	classNormalizationFilter->addConstructor< float, float, float >();
	classNormalizationFilter->addProperty("targetEnergy", &NormalizationFilter::getTargetEnergy);
	classNormalizationFilter->addProperty("energyThreshold", &NormalizationFilter::getEnergyThreshold);
	classNormalizationFilter->addProperty("attackRate", &NormalizationFilter::getAttackRate);
	registrar->registerClass(classNormalizationFilter);

	auto classRingModulationFilter = new AutoRuntimeClass< RingModulationFilter >();
	classRingModulationFilter->addConstructor();
	classRingModulationFilter->addConstructor< uint32_t >();
	classRingModulationFilter->addConstructor< uint32_t, uint32_t >();
	registrar->registerClass(classRingModulationFilter);

	auto classSurroundFilter = new AutoRuntimeClass< SurroundFilter >();
	classSurroundFilter->addConstructor< SurroundEnvironment*, const Vector4&, float >();
	classSurroundFilter->addMethod("setSpeakerPosition", &SurroundFilter::setSpeakerPosition);
	classSurroundFilter->addMethod("setMaxDistance", &SurroundFilter::setMaxDistance);
	registrar->registerClass(classSurroundFilter);

	auto classSoundHandle = new AutoRuntimeClass< SoundHandle >();
	classSoundHandle->addMethod("stop", &SoundHandle::stop);
	classSoundHandle->addMethod("fadeOff", &SoundHandle::fadeOff);
	classSoundHandle->addMethod("isPlaying", &SoundHandle::isPlaying);
	classSoundHandle->addMethod("setVolume", &SoundHandle::setVolume);
	classSoundHandle->addMethod("setPitch", &SoundHandle::setPitch);
	classSoundHandle->addMethod("setPosition", &SoundHandle::setPosition);
	classSoundHandle->addMethod("setParameter", &SoundHandle::setParameter);
	registrar->registerClass(classSoundHandle);

	auto classSoundPlayer = new AutoRuntimeClass< SoundPlayer >();
	classSoundPlayer->addMethod< Ref< SoundHandle >, const Sound*, uint32_t >("play", &SoundPlayer::play);
	classSoundPlayer->addMethod< Ref< SoundHandle >, const Sound*, const Vector4&, uint32_t, bool >("play", &SoundPlayer::play);
	registrar->registerClass(classSoundPlayer);

	auto classBankBuffer = new AutoRuntimeClass< BankBuffer >();
	classBankBuffer->addConstructor< const RefArray< IGrain >& >();
	registrar->registerClass(classBankBuffer);

	auto classIGrain = new AutoRuntimeClass< IGrain >();
	registrar->registerClass(classIGrain);

	auto classIGrainData = new AutoRuntimeClass< IGrainData >();
	registrar->registerClass(classIGrainData);

	auto classBlendGrain = new AutoRuntimeClass< BlendGrain >();
	registrar->registerClass(classBlendGrain);

	auto classBlendGrainData = new AutoRuntimeClass< BlendGrainData >();
	classBlendGrainData->addConstructor();
	registrar->registerClass(classBlendGrainData);

	auto classEnvelopeGrain = new AutoRuntimeClass< EnvelopeGrain >();
	registrar->registerClass(classEnvelopeGrain);

	auto classEnvelopeGrainData = new AutoRuntimeClass< EnvelopeGrainData >();
	classEnvelopeGrainData->addConstructor();
	registrar->registerClass(classEnvelopeGrainData);

	auto classInLoopOutGrain = new AutoRuntimeClass< InLoopOutGrain >();
	registrar->registerClass(classInLoopOutGrain);

	auto classInLoopOutGrainData = new AutoRuntimeClass< InLoopOutGrainData >();
	classInLoopOutGrainData->addConstructor();
	registrar->registerClass(classInLoopOutGrainData);

	auto classMuteGrain = new AutoRuntimeClass< MuteGrain >();
	registrar->registerClass(classMuteGrain);

	auto classMuteGrainData = new AutoRuntimeClass< MuteGrainData >();
	classMuteGrainData->addConstructor();
	registrar->registerClass(classMuteGrainData);

	auto classPlayGrain = new AutoRuntimeClass< PlayGrain >();
	registrar->registerClass(classPlayGrain);

	auto classPlayGrainData = new AutoRuntimeClass< PlayGrainData >();
	classPlayGrainData->addConstructor();
	registrar->registerClass(classPlayGrainData);

	auto classRandomGrain = new AutoRuntimeClass< RandomGrain >();
	registrar->registerClass(classRandomGrain);

	auto classRandomGrainData = new AutoRuntimeClass< RandomGrainData >();
	classRandomGrainData->addConstructor();
	registrar->registerClass(classRandomGrainData);

	auto classRepeatGrain = new AutoRuntimeClass< RepeatGrain >();
	registrar->registerClass(classRepeatGrain);

	auto classRepeatGrainData = new AutoRuntimeClass< RepeatGrainData >();
	classRepeatGrainData->addConstructor();
	registrar->registerClass(classRepeatGrainData);

	auto classSequenceGrain = new AutoRuntimeClass< SequenceGrain >();
	registrar->registerClass(classSequenceGrain);

	auto classSequenceGrainData = new AutoRuntimeClass< SequenceGrainData >();
	classSequenceGrainData->addConstructor();
	registrar->registerClass(classSequenceGrainData);

	auto classSimultaneousGrain = new AutoRuntimeClass< SimultaneousGrain >();
	registrar->registerClass(classSimultaneousGrain);

	auto classSimultaneousGrainData = new AutoRuntimeClass< SimultaneousGrainData >();
	classSimultaneousGrainData->addConstructor();
	registrar->registerClass(classSimultaneousGrainData);

	auto classTriggerGrain = new AutoRuntimeClass< TriggerGrain >();
	registrar->registerClass(classTriggerGrain);

	auto classTriggerGrainData = new AutoRuntimeClass< TriggerGrainData >();
	classTriggerGrainData->addConstructor();
	registrar->registerClass(classTriggerGrainData);
}

}
