#import <GameKit/GameKit.h>
#include <iterator>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Event.h"
#include "Online/Gc/GcLeaderboards.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcLeaderboards", GcLeaderboards, ILeaderboardsProvider)

GcLeaderboards::GcLeaderboards(const std::list< std::wstring >& leaderboardIds)
:	m_leaderboardIds(leaderboardIds)
{
}

bool GcLeaderboards::enumerate(std::map< std::wstring, LeaderboardData >& outLeaderboards)
{
	__block Event* bevent;
	__block bool result;
	Event event;

	uint64_t handle = 0;
	for (std::list< std::wstring >::const_iterator i = m_leaderboardIds.begin(); i != m_leaderboardIds.end(); ++i)
	{
		LeaderboardData data;
		data.handle = handle++;
		data.score = 0;
		data.rank = 0;

		if ([GKLocalPlayer localPlayer].authenticated == YES)
		{
			bevent = &event;

			NSString* identifier = makeNSString(*i);
			if (!identifier)
				return false;

			NSLog(@"loadScoresWithCompletionHandler of leaderboard \"%@\"...\n", identifier);

			GKLeaderboard* lb = [[GKLeaderboard alloc] init];
			lb.identifier = identifier;
			[lb loadScoresWithCompletionHandler:^(NSArray* scores, NSError* error)
			{
				if (error == nil)
				{
					result = true;
				}
				else
				{
					log::error << L"Failed to enumerate leaderboard;" << Endl;
					log::error << fromNSString([error localizedDescription]) << Endl;
					result = false;
				}
				bevent->broadcast();
			}];

			if (!event.wait(10000))
			{
				log::error << L"Failed to enumerate leaderboards; No response when loading scores of " << *i << Endl;
				return false;
			}

			event.reset();

			if (result)
				data.score = int32_t(lb.localPlayerScore.value);
		}

		outLeaderboards[*i] = data;
	}

	return true;
}

bool GcLeaderboards::create(const std::wstring& leaderboardId, LeaderboardData& outLeaderboard)
{
	return false;
}

bool GcLeaderboards::set(const uint64_t handle, int32_t score)
{
	__block Event* bevent;
	__block bool result;
	Event event;

	std::list< std::wstring >::const_iterator i = m_leaderboardIds.begin();
	std::advance(i, handle);

    GKScore* gks = [[GKScore alloc] initWithLeaderboardIdentifier: makeNSString(*i)];
    gks.value = score;
 
	bevent = &event;
   
    [GKScore reportScores:@[gks] withCompletionHandler:^(NSError *error)
    {
        if (error == nil)
        {
        	result = true;
        }
        else
        {
        	log::error << L"Failed to upload score to leaderboard" << Endl;
        	result = false;
        }
        bevent->broadcast();
    }];

    if (!event.wait(10000))
    {
    	log::error << L"Failed to upload score; No response when uploading score" << Endl;
    	return false;
    }

	return result;
}

bool GcLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

bool GcLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< ScoreData >& outScores)
{
	return false;
}

	}
}
