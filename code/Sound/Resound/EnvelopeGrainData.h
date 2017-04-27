/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_EnvelopeGrainData_H
#define traktor_sound_EnvelopeGrainData_H

#include "Core/RefArray.h"
#include "Sound/Types.h"
#include "Sound/Resound/IGrainData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS EnvelopeGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	struct GrainData
	{
		Ref< IGrainData > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;

		GrainData()
		:	in(0.0f)
		,	out(0.0f)
		,	easeIn(0.2f)
		,	easeOut(0.2f)
		{
		}

		void serialize(ISerializer& s);
	};

	EnvelopeGrainData();

	void addGrain(IGrainData* grain, float in, float out, float easeIn, float easeOut);

	void removeGrain(IGrainData* grain);

	void setLevels(const float levels[3]);

	void setMid(float mid);

	void setResponse(float response);

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::wstring& getId() const { return m_id; }

	const std::vector< GrainData >& getGrains() const { return m_grains; }

	const float* getLevels() const { return m_levels; }

	float getMid() const { return m_mid; }

	float getResponse() const { return m_response; }

private:
	std::wstring m_id;
	std::vector< GrainData > m_grains;
	float m_levels[3];
	float m_mid;
	float m_response;
};

	}
}

#endif	// traktor_sound_EnvelopeGrainData_H
