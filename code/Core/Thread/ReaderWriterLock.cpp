/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/ReaderWriterLock.h"

namespace traktor
{

bool ReaderWriterLock::acquireReader(int32_t timeout)
{
	m_lock.lock_shared();
	return true;
}

void ReaderWriterLock::releaseReader()
{
	m_lock.unlock_shared();
}

bool ReaderWriterLock::acquireWriter(int32_t timeout)
{
	m_lock.lock();
	return true;
}

void ReaderWriterLock::releaseWriter()
{
	m_lock.unlock();
}

}
