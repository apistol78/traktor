/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/BitReader.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/Encoders/TssStreamEncoder.h"
#include "Sound/Editor/Tracker/ImportMod.h"
#include "Sound/Editor/Tracker/SongAsset.h"
#include "Sound/Filters/LowPassFilter.h"
#include "Sound/Filters/TimeStretchFilter.h"
#include "Sound/Tracker/GotoEventData.h"
#include "Sound/Tracker/TrackData.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Tracker/PlayData.h"
#include "Sound/Tracker/SetBpmEventData.h"
#include "Sound/Tracker/VolumeEventData.h"
#include "Sound/Tracker/VolumeSlideEventData.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{


const int c_periodNote[][2] =
{
	{ 856, 12, },	// C1
	{ 808, 13, },	// C#1
	{ 762, 14, },	// D1
	{ 720, 15, },	// D#1
	{ 678, 16, },	// E1
	{ 640, 17, },	// F1
	{ 604, 18, },	// F#1
	{ 570, 19, },	// G1
	{ 538, 20, },	// G#1
	{ 508, 21, },	// A1
	{ 480, 22, },	// A#1
	{ 453, 23, },	// B1

	{ 428, 24, },	// C2
	{ 404, 25, },	// C#2
	{ 381, 26, },	// D2
	{ 360, 27, },	// D#2
	{ 339, 28, },	// E2
	{ 320, 29, },	// F2
	{ 302, 30, },	// F#2
	{ 285, 31, },	// G2
	{ 269, 32, },	// G#2
	{ 254, 33, },	// A2
	{ 240, 34, },	// A#2
	{ 226, 35, },	// B2

	{ 214, 36, },	// C3
	{ 202, 37, },	// C#3
	{ 190, 38, },	// D3
	{ 180, 39, },	// D#3
	{ 170, 40, },	// E3
	{ 160, 41, },	// F3
	{ 151, 42, },	// F#3
	{ 143, 43, },	// G3
	{ 135, 44, },	// G#3
	{ 127, 45, },	// A3
	{ 120, 46, },	// A#3
	{ 113, 47, },	// B3

	{ 0, 0, }	// C2
};


		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.ImportMod", ImportMod, Object)

bool ImportMod::import(const Path& fileName, const Path& assetPath, const Path& samplePath, db::Group* group) const
{
	char str[64] = { 0 };

	Ref< IStream > fs = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!fs)
		return false;

	fs->seek(IStream::SeekCurrent, 20);

	BitReader br(fs);

	struct SampleHeader
	{
		uint16_t length;
		uint8_t fineTune;
		uint8_t volume;
		uint16_t repeatOffset;
		uint16_t repeatLength;
	};

	SampleHeader sampleHeaders[31] = { 0 };
	for (int32_t i = 0; i < 31; ++i)
	{
		fs->read(str, 22);
		sampleHeaders[i].length = br.readUInt16();
		sampleHeaders[i].fineTune = br.readUInt8();
		sampleHeaders[i].volume = br.readUInt8();
		sampleHeaders[i].repeatOffset = br.readUInt16();
		sampleHeaders[i].repeatLength = br.readUInt16();

		swap8in16(sampleHeaders[i].length);
 		swap8in16(sampleHeaders[i].repeatOffset);
		swap8in16(sampleHeaders[i].repeatLength);
	}

	const uint8_t songLength = br.readUInt8();
	const uint8_t bpm = br.readUInt8();

	uint8_t sequence[128];
	fs->read(sequence, 128);

	char magic[5] = { 0 };
	fs->read(magic, 4);
	log::info << L"Magic " << mbstows(magic) << Endl;


	uint8_t maxPattern = 0;
	for (uint32_t i = 0; i < 128; ++i)
	{
		if (sequence[i] > maxPattern)
			maxPattern = sequence[i];
	}

	uint8_t patterns[256][1024];
	for (uint32_t i = 0; i <= maxPattern; ++i)
		fs->read(patterns[i], 1024);

	// Create sounds for each sample used in MOD.
	Guid soundIds[31];

	for (int32_t i = 0; i < 31; ++i)
	{
		if (sampleHeaders[i].length == 0)
			continue;

		AlignedVector< int8_t > data(sampleHeaders[i].length * 2);
		fs->read(data.ptr(), sampleHeaders[i].length * 2);

		AlignedVector< float > fd(data.size());
		for (uint32_t j = 0; j < data.size(); ++j)
		{
			int32_t v = (int32_t)data[j];
			fd[j] = (v / 128.0f) * sampleHeaders[i].volume / 64.0f;
		}

		// Clear first sample since it's been used for repeat information.
		fd[0] = 0.0f;
		fd[1] = 0.0f;

		Ref< IStream > fos = FileSystem::getInstance().open(assetPath.getPathName() + L"/" + samplePath.getPathName() + L"/Instrument_" + toString(i) + L".tss", File::FmWrite);

		TssStreamEncoder ose;
		ose.create(fos);

		SoundBlock sb = { 0 };
		sb.samples[0] = fd.ptr();
		sb.samplesCount = fd.size();
		sb.sampleRate = uint32_t(8287 * 6.726800183);
		sb.maxChannel = 1;

		// Strech samples, to give room for some filtering.
		TimeStretchFilter stretchFlter(4.0f);
		Ref< IFilterInstance > stretchFilterInstance = stretchFlter.createInstance();
		stretchFlter.apply(stretchFilterInstance, sb);
		sb.sampleRate *= 4;

		LowPassFilter lowPassFilter(8287.0f);
		Ref< IFilterInstance > lowPassFilterInstance = lowPassFilter.createInstance();
		lowPassFilter.apply(lowPassFilterInstance, sb);

		// Encode sound block.
		ose.putBlock(sb);
		ose.destroy();

		fos->close();

		Ref< SoundAsset > soundAsset = new SoundAsset();
		soundAsset->setFileName(samplePath.getPathName() + L"/Instrument_" + toString(i) + L".tss");
		soundAsset->setStream(false);
		soundAsset->setPreload(true);
		soundAsset->setCompressed(false);

		Ref< db::Instance > soundInstance = group->createInstance(L"Instrument_" + toString(i), db::CifReplaceExisting | db::CifKeepExistingGuid);
		soundInstance->setObject(soundAsset);
		soundInstance->commit();

		soundIds[i] = soundInstance->getGuid();
	}


	// Create song.
	Ref< SongAsset > songAsset = new SongAsset();
	songAsset->setBpm(125);


	for (uint32_t i = 0; i < songLength; ++i)
	{
		const uint8_t* pattern = patterns[sequence[i]];

		Ref< TrackData > td[4];
		td[0] = new TrackData();
		td[1] = new TrackData();
		td[2] = new TrackData();
		td[3] = new TrackData();

		for (uint32_t row = 0; row < 64; ++row)
		{
			for (uint32_t chan = 0; chan < 4; ++chan)
			{
				TrackData::Key k;
				k.at = row;

				const uint8_t a = pattern[(row * 4 + chan) * 4 + 0];
				const uint8_t b = pattern[(row * 4 + chan) * 4 + 1];
				const uint8_t c = pattern[(row * 4 + chan) * 4 + 2];
				const uint8_t d = pattern[(row * 4 + chan) * 4 + 3];

				const uint8_t sample = (a & 0xf0) | ((c & 0xf0) >> 4);
				if (sample)
				{
					const uint16_t period = ((a & 0x0f) << 8) | b;
					for (uint32_t i = 0; c_periodNote[i][0] != 0; ++i)
					{
						if (c_periodNote[i][0] == period)
						{
							k.play = new PlayData(
								resource::Id< Sound >(soundIds[sample - 1]),
								c_periodNote[i][1],
								int32_t(sampleHeaders[sample - 1].repeatOffset) * 2,
								int32_t(sampleHeaders[sample - 1].repeatLength) * 2
							);
							break;
						}
					}
					if (!k.play)
						log::warning << L"Unable to find note for period " << int(period) << Endl;
				}

				const uint16_t effect = ((c & 0x0f) << 8) | d;
				if (effect)
				{
					const uint8_t f = effect >> 8;
					const uint16_t x = (effect >> 4) & 0x0f;
					const uint16_t y = effect & 0x0f;

					log::info << L"Row " << row << L" | Channel " << chan << L"; effect " << int(f) << L", x " << int(x) << L", y " << int(y) << Endl;

					switch (f)
					{
					case 10:
						{
							if (x > 0)
								k.events.push_back(new VolumeSlideEventData( float(x) / 64.0f));
							else
								k.events.push_back(new VolumeSlideEventData(-float(y) / 64.0f));
						}
						break;

					case 11:
						//k.events.push_back(new Goto)
						break;

					case 13:
						k.events.push_back(new GotoEventData(i + 1, 0));
						break;

					case 12:
						{
							const float volume = (x * 16 + y) / 64.0f;
							k.events.push_back(new VolumeEventData(volume));
						}
						break;

					case 15:
						{
							const uint16_t speed = x * 16 + y;
							if (speed >= 32)
								k.events.push_back(new SetBpmEventData(speed));
						}
						break;

					default:
						log::warning << L"Effect " << f << L" not implemented." << Endl;
						break;
					}
				}

				if (k.play != nullptr || !k.events.empty())
					td[chan]->addKey(k);

			}
		}

		Ref< PatternData > pd = new PatternData(64);
		pd->addTrack(td[0]);
		pd->addTrack(td[1]);
		pd->addTrack(td[2]);
		pd->addTrack(td[3]);

		songAsset->addPattern(pd);
	}

	Ref< db::Instance > songInstance = group->createInstance(L"Song", db::CifReplaceExisting | db::CifKeepExistingGuid);
	songInstance->setObject(songAsset);
	songInstance->commit();

	return true;
}

	}
}
