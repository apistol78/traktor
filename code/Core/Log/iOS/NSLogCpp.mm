#include <Foundation/Foundation.h>
#include <cwchar>

namespace traktor
{
	
void NSLogCpp(const wchar_t* s)
{
	NSString* nss = [[NSString alloc] initWithBytes:s length:wcslen(s)*sizeof(*s) encoding:NSUTF32LittleEndianStringEncoding];
	NSLog(nss);
	[nss release];
}

}
