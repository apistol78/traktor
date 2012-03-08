#import <GameKit/GameKit.h>
#include "Online/Gc/GcAchievements.h"

namespace traktor
{
	namespace online
	{

NSString* makeNSString(const std::wstring& str)
{
	return [[[NSString alloc] initWithBytes: str.c_str() length: str.length() encoding: NSUTF32StringEncoding] autorelease];
}

std::wstring fromNSString(const NSString* str)
{
	wchar_t buffer[4096];
	[str getCString: (char*)buffer maxLength: sizeof_array(buffer) encoding: NSUTF32StringEncoding];
	return std::wstring(buffer);
}

class GcAchievementsImpl : public Object
{
public:
	GcAchievementsImpl()
	:	m_achievements(0)
	{
	}

	bool enumerate(std::map< std::wstring, bool >& outAchievements)
	{
		// Load a local cache of all achievements.
		if (!m_achievements)
		{
			[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray* achievements, NSError* error)
			{
				m_achievements = achievements;
			}];
		}
		
		// Enumerate all achievements.
		for (GKAchievement* achievement in m_achievements)
		{
			outAchievements.insert(std::make_pair(
				fromNSString(achievement.identifier),
				achievement.percentComplete >= 99.0f
			));
		}
		
		return true;
	}
	
	bool set(const std::wstring& achievementId, bool reward)
	{
		for (GKAchievement* achievement in m_achievements)
		{
			if (achievementId == fromNSString(achievement.identifier))
			{
				achievement.percentComplete = reward ? 100.0f : 0.0f;
				[achievement reportAchievementWithCompletionHandler:^(NSError *error) {
					// \fixme
				}];
				return true;
			}
		}
		return false;
	}

private:
	NSArray* m_achievements;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcAchievements", GcAchievements, IAchievementsProvider)

GcAchievements::GcAchievements()
:	m_impl(new GcAchievementsImpl())
{
}

bool GcAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	return m_impl->enumerate(outAchievements);
}

bool GcAchievements::set(const std::wstring& achievementId, bool reward)
{
	return m_impl->set(achievementId, reward);
}

	}
}
