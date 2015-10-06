#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Sound/SoundClassFactory.h"
#	include "Sound/StaticSoundResource.h"
#	include "Sound/StreamSoundResource.h"
#	include "Sound/Decoders/FlacStreamDecoder.h"
#	include "Sound/Decoders/Mp3StreamDecoder.h"
#	include "Sound/Decoders/OggStreamDecoder.h"
#	include "Sound/Decoders/WavStreamDecoder.h"
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
