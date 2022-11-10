/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "Ui/Cocoa/NSTargetProxy.h"

@implementation NSTargetProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_callback = 0;
	return self;
}

- (void) setCallback: (traktor::ui::ITargetProxyCallback*)callback
{
	m_callback = callback;
}

- (void) dispatchActionCallback: (id)controlId
{
	m_callback->targetProxy_Action(controlId);
}

- (void) dispatchDoubleActionCallback: (id)controlId
{
	m_callback->targetProxy_doubleAction(controlId);
}

@end
