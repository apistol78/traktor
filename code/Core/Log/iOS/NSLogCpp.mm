/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
