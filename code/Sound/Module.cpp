#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/SoundClassFactory.h"
#	include "Sound/StaticSoundResource.h"
#	include "Sound/StreamSoundResource.h"
#	include "Sound/Decoders/FlacStreamDecoder.h"
#	include "Sound/Decoders/Mp3StreamDecoder.h"
#	include "Sound/Decoders/OggStreamDecoder.h"
#	include "Sound/Decoders/WavStreamDecoder.h"

// Processor
#	include "Sound/Processor/GraphResource.h"
#	include "Sound/Processor/Nodes/Add.h"
#	include "Sound/Processor/Nodes/Blend.h"
#	include "Sound/Processor/Nodes/Divide.h"
#	include "Sound/Processor/Nodes/Modulate.h"
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

namespace traktor
{
	namespace sound
	{

extern "C" void __module__Traktor_Sound()
{
	T_FORCE_LINK_REF(SoundClassFactory);
	T_FORCE_LINK_REF(StaticSoundResource);
	T_FORCE_LINK_REF(StreamSoundResource);
	T_FORCE_LINK_REF(FlacStreamDecoder);
	T_FORCE_LINK_REF(Mp3StreamDecoder);
	T_FORCE_LINK_REF(OggStreamDecoder);
	T_FORCE_LINK_REF(WavStreamDecoder);

	// Processor
	T_FORCE_LINK_REF(GraphResource);
	T_FORCE_LINK_REF(Add);
	T_FORCE_LINK_REF(Blend);
	T_FORCE_LINK_REF(Divide);
	T_FORCE_LINK_REF(Modulate);
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
}

	}
}

#endif
