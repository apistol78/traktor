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
	std::string mbs = wstombs(Utf8Encoding(), str);
	return [[[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding] autorelease];
}

std::wstring fromNSString(const NSString* str)
{
	char buffer[4096];
	[str getCString: buffer maxLength: sizeof_array(buffer) encoding: NSUTF8StringEncoding];
	return mbstows(buffer);
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

		bevent = &event;

		GKLeaderboard* lb = [[GKLeaderboard alloc] init];
		lb.identifier = makeNSString(*i);
		[lb loadScoresWithCompletionHandler:^(NSArray* scores, NSError* error)
		{
			if (error == nil)
			{
				result = true;
			}
			else
			{
				log::error << L"Failed to download leaderboard!" << Endl;
				result = false;
			}
			bevent->broadcast();
		}];

		if (!event.wait(10000))
		{
			log::error << L"Failed to enumerate leaderboards; No response when loading scores of " << *i << Endl;
			return false;
		}

		if (result)
		{
			data.score = int32_t(lb.localPlayerScore.value);
			outLeaderboards[*i] = data;
		}
		else
			return false;

		event.reset();
	}

	return true;
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

bool GcLeaderboards::getGlobalScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	return false;
}

bool GcLeaderboards::getFriendScores(uint64_t handle, int32_t from, int32_t to, std::vector< std::pair< uint64_t, int32_t > >& outScores)
{
	return false;
}

	}
}
