/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/AudioClassFactory.h"
#	include "Sound/StaticAudioResource.h"
#	include "Sound/StreamAudioResource.h"
#	include "Sound/Decoders/FlacStreamDecoder.h"
#	include "Sound/Decoders/Mp3StreamDecoder.h"
#	include "Sound/Decoders/OggStreamDecoder.h"
#	include "Sound/Decoders/WavStreamDecoder.h"

// Processor
#	include "Sound/Processor/Edge.h"
#	include "Sound/Processor/Graph.h"
#	include "Sound/Processor/GraphResource.h"
#	include "Sound/Processor/InputPin.h"
#	include "Sound/Processor/OutputPin.h"
#	include "Sound/Processor/Nodes/Add.h"
#	include "Sound/Processor/Nodes/Blend.h"
#	include "Sound/Processor/Nodes/Custom.h"
#	include "Sound/Processor/Nodes/Divide.h"
#	include "Sound/Processor/Nodes/Filter.h"
#	include "Sound/Processor/Nodes/Multiply.h"
#	include "Sound/Processor/Nodes/Output.h"
#	include "Sound/Processor/Nodes/Parameter.h"
#	include "Sound/Processor/Nodes/Pitch.h"
#	include "Sound/Processor/Nodes/Scalar.h"
#	include "Sound/Processor/Nodes/Sine.h"
#	include "Sound/Processor/Nodes/Source.h"
#	include "Sound/Processor/Nodes/Subtract.h"
#	include "Sound/Processor/Nodes/Time.h"

// Resound
#	include "Sound/Resound/BankResource.h"
#	include "Sound/Resound/BlendGrainData.h"
#	include "Sound/Resound/EnvelopeGrainData.h"
#	include "Sound/Resound/InLoopOutGrainData.h"
#	include "Sound/Resound/MuteGrainData.h"
#	include "Sound/Resound/PlayGrainData.h"
#	include "Sound/Resound/RandomGrainData.h"
#	include "Sound/Resound/RepeatGrainData.h"
#	include "Sound/Resound/SequenceGrainData.h"
#	include "Sound/Resound/SimultaneousGrainData.h"
#	include "Sound/Resound/TriggerGrainData.h"

// Filters
#	include "Sound/Filters/CombFilter.h"
#	include "Sound/Filters/DitherFilter.h"
#	include "Sound/Filters/EchoFilter.h"
#	include "Sound/Filters/EqualizerFilter.h"
#	include "Sound/Filters/FFTFilter.h"
#	include "Sound/Filters/GroupFilter.h"
#	include "Sound/Filters/LowPassFilter.h"
#	include "Sound/Filters/NormalizationFilter.h"
#	include "Sound/Filters/ReverbFilter.h"
#	include "Sound/Filters/RingModulationFilter.h"
#	include "Sound/Filters/SurroundEnvironment.h"
#	include "Sound/Filters/SurroundFilter.h"
#	include "Sound/Filters/TimeStretchFilter.h"

namespace traktor::sound
{

extern "C" void __module__Traktor_Sound()
{
	T_FORCE_LINK_REF(AudioClassFactory);
	T_FORCE_LINK_REF(StaticAudioResource);
	T_FORCE_LINK_REF(StreamAudioResource);
	T_FORCE_LINK_REF(FlacStreamDecoder);
	T_FORCE_LINK_REF(Mp3StreamDecoder);
	T_FORCE_LINK_REF(OggStreamDecoder);
	T_FORCE_LINK_REF(WavStreamDecoder);

	// Processor
	T_FORCE_LINK_REF(Edge);
	T_FORCE_LINK_REF(Graph);
	T_FORCE_LINK_REF(GraphResource);
	T_FORCE_LINK_REF(InputPin);
	T_FORCE_LINK_REF(OutputPin);
	T_FORCE_LINK_REF(Add);
	T_FORCE_LINK_REF(Blend);
	T_FORCE_LINK_REF(Custom);
	T_FORCE_LINK_REF(Divide);
	T_FORCE_LINK_REF(Filter);
	T_FORCE_LINK_REF(Multiply);
	T_FORCE_LINK_REF(Output);
	T_FORCE_LINK_REF(Parameter);
	T_FORCE_LINK_REF(Pitch);
	T_FORCE_LINK_REF(Scalar);
	T_FORCE_LINK_REF(Sine);
	T_FORCE_LINK_REF(Source);
	T_FORCE_LINK_REF(Subtract);
	T_FORCE_LINK_REF(Time);

	// Resound
	T_FORCE_LINK_REF(BankResource);
	T_FORCE_LINK_REF(BlendGrainData);
	T_FORCE_LINK_REF(EnvelopeGrainData);
	T_FORCE_LINK_REF(InLoopOutGrainData);
	T_FORCE_LINK_REF(MuteGrainData);
	T_FORCE_LINK_REF(PlayGrainData);
	T_FORCE_LINK_REF(RandomGrainData);
	T_FORCE_LINK_REF(RepeatGrainData);
	T_FORCE_LINK_REF(SequenceGrainData);
	T_FORCE_LINK_REF(SimultaneousGrainData);
	T_FORCE_LINK_REF(TriggerGrainData);

	// Filters
	T_FORCE_LINK_REF(CombFilter);
	T_FORCE_LINK_REF(DitherFilter);
	T_FORCE_LINK_REF(EchoFilter);
	T_FORCE_LINK_REF(EqualizerFilter);
	T_FORCE_LINK_REF(FFTFilter);
	T_FORCE_LINK_REF(GroupFilter);
	T_FORCE_LINK_REF(LowPassFilter);
	T_FORCE_LINK_REF(NormalizationFilter);
	T_FORCE_LINK_REF(ReverbFilter);
	T_FORCE_LINK_REF(RingModulationFilter);
	T_FORCE_LINK_REF(SurroundEnvironment);
	T_FORCE_LINK_REF(SurroundFilter);
	T_FORCE_LINK_REF(TimeStretchFilter);
}

}

#endif
