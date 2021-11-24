#pragma once

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class PatternData;

class T_DLLCLASS SongAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	SongAsset();

	void setBpm(int32_t bpm);

	int32_t getBpm() const { return m_bpm; }

	void addPattern(const PatternData* pattern);

	const RefArray< const PatternData >& getPatterns() const { return m_patterns; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SongPipeline;

	Guid m_category;
	float m_presence;
	float m_presenceRate;
	int32_t m_bpm;
	RefArray< const PatternData > m_patterns;
};

	}
}
