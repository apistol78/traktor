#ifndef traktor_sound_BlendGrainData_H
#define traktor_sound_BlendGrainData_H

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

class T_DLLCLASS BlendGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	const std::wstring& getId() const { return m_id; }

	const Ref< IGrainData >* getGrains() const { return m_grains; }

private:
	std::wstring m_id;
	Ref< IGrainData > m_grains[2];
};

	}
}

#endif	// traktor_sound_BlendGrainData_H
