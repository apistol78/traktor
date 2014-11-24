#import <GameKit/GameKit.h>
#include "Core/Log/Log.h"
#include "Core/Thread/Event.h"
#include "Online/Gc/GcAchievements.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

NSString* makeNSString(const std::wstring& str)
{
	return [[[NSString alloc] initWithBytes: str.data() length: str.size() * sizeof(wchar_t) encoding: NSUTF32LittleEndianStringEncoding] autorelease];
}

std::wstring fromNSString(const NSString* str)
{
	wchar_t buffer[4096];
	[str getCString: (char*)buffer maxLength: sizeof_array(buffer) encoding: NSUTF32LittleEndianStringEncoding];
	return std::wstring(buffer);
}

		}

class GcAchievementsImpl : public Object
{
public:
	GcAchievementsImpl(const std::list< std::wstring >& achievementIds)
	:	m_achievementIds(achievementIds)
	{
	}

	bool enumerate(std::map< std::wstring, bool >& outAchievements)
	{
		__block std::map< std::wstring, bool >* boutAchievements;
		__block Event* bevent;
		Event event;

		for (std::list< std::wstring >::const_iterator i = m_achievementIds.begin(); i != m_achievementIds.end(); ++i)
			outAchievements[*i] = false;

		if ([GKLocalPlayer localPlayer].authenticated == NO)
			return true;

		boutAchievements = &outAchievements;
		bevent = &event;

		[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray* achievements, NSError* error)
		{
			if (error == nil)
			{
				for (GKAchievement* achievement in achievements)
				{
					std::wstring id = fromNSString(achievement.identifier);
					(*boutAchievements)[id] = bool(achievement.percentComplete >= 99.0f);
					log::info << L"Enum achievement \"" << id << L"\", percent " << float(achievement.percentComplete) << Endl;
				}				
			}
			else
			{
				log::error << L"Failed to enumerate achievements;" << Endl;				
				log::error << fromNSString([error localizedDescription]) << Endl;
			}
			bevent->broadcast();
		}];

		if (!event.wait(10000))
		{
			log::error << L"Failed to enumerate achievements; No response when download achievements" << Endl;
			return false;
		}
		
		return true;
	}
	
	bool set(const std::wstring& achievementId, bool reward)
	{
		__block Event* bevent;
		__block bool result;
		Event event;

		GKAchievement* achievement = [[GKAchievement alloc] initWithIdentifier: makeNSString(achievementId)];
		achievement.showsCompletionBanner = reward ? YES : NO;
		achievement.percentComplete = reward ? 100.0f : 0.0f;

		bevent = &event;
		[achievement reportAchievementWithCompletionHandler:^(NSError *error) {
			result = bool(error == nil);
			bevent->broadcast();
		}];
        
        if (!event.wait(100000))
        {
            log::error << L"Failed to reward achievement; No response when rewarding achievement \"" << achievementId << L"\"" << Endl;
            return false;
        }

		return result;
	}

private:
	std::list< std::wstring > m_achievementIds;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcAchievements", GcAchievements, IAchievementsProvider)

GcAchievements::GcAchievements(const std::list< std::wstring >& achievementIds)
:	m_impl(new GcAchievementsImpl(achievementIds))
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
