#ifndef traktor_sound_InLoopOutGrainData_H
#define traktor_sound_InLoopOutGrainData_H

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

class T_DLLCLASS InLoopOutGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	InLoopOutGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const;

	virtual void serialize(ISerializer& s);

	const std::wstring& getId() const { return m_id; }

	IGrainData* getInGrain() const { return m_inGrain; }

	IGrainData* getInLoopGrain() const { return m_inLoopGrain; }

	IGrainData* getOutGrain() const { return m_outGrain; }

	IGrainData* getOutLoopGrain() const { return m_outLoopGrain; }

private:
	std::wstring m_id;
	bool m_inital;
	Ref< IGrainData > m_inGrain;
	Ref< IGrainData > m_inLoopGrain;
	Ref< IGrainData > m_outGrain;
	Ref< IGrainData > m_outLoopGrain;
};

	}
}

#endif	// traktor_sound_InLoopOutGrainData_H
