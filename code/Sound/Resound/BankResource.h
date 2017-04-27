/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_BankResource_H
#define traktor_sound_BankResource_H

#include "Core/RefArray.h"
#include "Sound/ISoundResource.h"

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

class IGrainData;

/*! \brief Bank sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS BankResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	BankResource();

	BankResource(const RefArray< IGrainData >& grains, const std::wstring& category, float gain, float presence, float presenceRate, float range);

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const RefArray< IGrainData >& getGrains() const { return m_grains; }

private:
	RefArray< IGrainData > m_grains;
	std::wstring m_category;
	float m_gain;
	float m_presence;
	float m_presenceRate;
	float m_range;
};

	}
}

#endif	// traktor_sound_BankResource_H
