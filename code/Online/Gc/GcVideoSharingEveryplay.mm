#import <Everyplay/Everyplay.h>
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Online/Gc/GcGameConfiguration.h"
#include "Online/Gc/GcVideoSharingEveryplay.h"

using namespace traktor;

namespace
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

}

@interface EveryplayCallback : NSObject< EveryplayDelegate >
{
@public
	Ref< const PropertyGroup > m_metaData;
}

@end

@implementation EveryplayCallback

- (void)everyplayShown
{
}

- (void)everyplayHidden
{
}

- (void)everyplayReadyForRecording:(NSNumber *)enabled
{
}

- (void)everyplayRecordingStarted
{
}

- (void)everyplayRecordingStopped
{
	if (m_metaData)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = m_metaData->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			NSString* key = makeNSString(i->first);

			if (const PropertyInteger* propInt = dynamic_type_cast< const PropertyInteger* >(i->second))
			{
				NSNumber* value = [[NSNumber numberWithInt: PropertyInteger::get(propInt)] autorelease];
				[[Everyplay sharedInstance] mergeSessionDeveloperData:@{ key : value }];
			}
			else if (const PropertyString* propStr = dynamic_type_cast< const PropertyString* >(i->second))
			{
				NSString* value = makeNSString(PropertyString::get(propStr));
				[[Everyplay sharedInstance] mergeSessionDeveloperData:@{ key : value }];
			}
		}
		m_metaData = 0;
	}
}

- (void)everyplayFaceCamSessionStarted
{
}

- (void)everyplayFaceCamRecordingPermission:(NSNumber *)granted
{
}

- (void)everyplayFaceCamSessionStopped
{
}

- (void)everyplayUploadDidStart:(NSNumber *)videoId
{
}

- (void)everyplayUploadDidProgress:(NSNumber *)videoId progress:(NSNumber *)progress
{
}

- (void)everyplayUploadDidComplete:(NSNumber *)videoId
{
}

- (void)everyplayThumbnailReadyAtFilePath:(NSString *)thumbnailFilePath
{
}

- (void)everyplayThumbnailReadyAtURL:(NSURL *)thumbnailUrl
{
}

- (void)everyplayThumbnailReadyAtTextureId:(NSNumber *)textureId portraitMode:(NSNumber *)portrait
{
}

@end

namespace traktor
{
	namespace online
	{
		namespace
		{

EveryplayCallback* s_callback = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcVideoSharingEveryplay", GcVideoSharingEveryplay, GcVideoSharing)

bool GcVideoSharingEveryplay::create(const GcGameConfiguration& configuration)
{
	@try
	{
		[Everyplay
			setClientId: @"6a5a429635aac0766f5edeceade6f93d8facada4" //makeNSString(configuration.m_sharingClientId)
			clientSecret: @"44e5c52b03df64685a36b77ae3c78d495c1e7f80" //makeNSString(configuration.m_sharingClientSecret)
			redirectURI: @"https://m.everyplay.com/auth" //makeNSString(configuration.m_sharingRedirectURI)
		];
	}
	@catch (NSException* exception)
	{
		log::error << L"Unable to initialize Everyplay; incorrect configuration." << Endl;
		return false;
	}

	s_callback = [[EveryplayCallback alloc] init];

	UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
	[Everyplay initWithDelegate: s_callback andParentViewController: rootViewController];

	return true;
}

bool GcVideoSharingEveryplay::beginCapture(int32_t duration)
{
	if (s_callback)
	{
		[[[Everyplay sharedInstance] capture] startRecording];
		return true;
	}
	else
		return false;
}

void GcVideoSharingEveryplay::endCapture(const PropertyGroup* metaData)
{
	if (s_callback)
	{
		s_callback->m_metaData = metaData;
		[[[Everyplay sharedInstance] capture] stopRecording];
	}
}

bool GcVideoSharingEveryplay::showShareUI()
{
	if (s_callback)
	{
		[[Everyplay sharedInstance] showEveryplaySharingModal];
		return true;
	}
	else
		return false;
}

	}
}
