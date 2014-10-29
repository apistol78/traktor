#import <Everyplay/Everyplay.h>
#include "Core/Log/Log.h"
#include "Online/Gc/GcGameConfiguration.h"
#include "Online/Gc/GcVideoSharingEveryplay.h"

using namespace traktor;

@interface EveryplayCallback : NSObject< EveryplayDelegate >

@end

@implementation EveryplayCallback

- (void)everyplayShown
{
	log::info << L"EveryplayCallback -- everyplayShown" << Endl;
}

- (void)everyplayHidden
{
	log::info << L"EveryplayCallback -- everyplayHidden" << Endl;
}

- (void)everyplayReadyForRecording:(NSNumber *)enabled
{
	log::info << L"EveryplayCallback -- everyplayReadyForRecording" << Endl;
}

- (void)everyplayRecordingStarted
{
	log::info << L"EveryplayCallback -- everyplayRecordingStarted" << Endl;
}

- (void)everyplayRecordingStopped
{
	log::info << L"EveryplayCallback -- everyplayRecordingStopped" << Endl;
	//[[Everyplay sharedInstance] mergeSessionDeveloperData:@{@"testString" : @"hello"}];
	//[[Everyplay sharedInstance] mergeSessionDeveloperData:@{@"testInteger" : @42}];
}

- (void)everyplayFaceCamSessionStarted
{
	log::info << L"EveryplayCallback -- everyplayFaceCamSessionStarted" << Endl;
}

- (void)everyplayFaceCamRecordingPermission:(NSNumber *)granted
{
	log::info << L"EveryplayCallback -- everyplayFaceCamRecordingPermission" << Endl;
}

- (void)everyplayFaceCamSessionStopped
{
	log::info << L"EveryplayCallback -- everyplayFaceCamSessionStopped" << Endl;
}

- (void)everyplayUploadDidStart:(NSNumber *)videoId
{
	log::info << L"EveryplayCallback -- everyplayUploadDidStart" << Endl;
}

- (void)everyplayUploadDidProgress:(NSNumber *)videoId progress:(NSNumber *)progress
{
	log::info << L"EveryplayCallback -- everyplayUploadDidProgress" << Endl;
}

- (void)everyplayUploadDidComplete:(NSNumber *)videoId
{
	log::info << L"EveryplayCallback -- everyplayUploadDidComplete" << Endl;
}

- (void)everyplayThumbnailReadyAtFilePath:(NSString *)thumbnailFilePath
{
	log::info << L"EveryplayCallback -- everyplayThumbnailReadyAtFilePath" << Endl;
}

- (void)everyplayThumbnailReadyAtURL:(NSURL *)thumbnailUrl
{
	log::info << L"EveryplayCallback -- everyplayThumbnailReadyAtURL" << Endl;
}

- (void)everyplayThumbnailReadyAtTextureId:(NSNumber *)textureId portraitMode:(NSNumber *)portrait
{
	log::info << L"EveryplayCallback -- everyplayThumbnailReadyAtTextureId" << Endl;
}

@end

namespace traktor
{
	namespace online
	{
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

EveryplayCallback* s_callback = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcVideoSharingEveryplay", GcVideoSharingEveryplay, GcVideoSharing)

bool GcVideoSharingEveryplay::create(const GcGameConfiguration& configuration)
{
	s_callback = [[EveryplayCallback alloc] init];

	[Everyplay
		setClientId: makeNSString(configuration.m_sharingClientId)
		clientSecret: makeNSString(configuration.m_sharingClientSecret)
		redirectURI: makeNSString(configuration.m_sharingRedirectURI)
	];

	UIViewController* rootViewController = [UIApplication sharedApplication].keyWindow.rootViewController;
	[Everyplay initWithDelegate: s_callback andParentViewController: rootViewController];

	return true;
}

bool GcVideoSharingEveryplay::beginCapture(int32_t duration)
{
	[[[Everyplay sharedInstance] capture] startRecording];
	return true;
}

void GcVideoSharingEveryplay::endCapture()
{
	[[[Everyplay sharedInstance] capture] stopRecording];
}

bool GcVideoSharingEveryplay::showShareUI()
{
	[[Everyplay sharedInstance] showEveryplaySharingModal];
	return true;
}

	}
}
