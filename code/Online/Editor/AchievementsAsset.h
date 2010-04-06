#ifndef traktor_online_AchievementsAsset_H
#define traktor_online_AchievementsAsset_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class AchievementDesc;

class T_DLLCLASS AchievementsAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void add(const AchievementDesc* achievement);

	const RefArray< const AchievementDesc >& get() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< const AchievementDesc > m_achievements;
};

	}
}

#endif	// traktor_online_AchievementsAsset_H
