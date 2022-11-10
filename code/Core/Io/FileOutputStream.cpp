/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileOutputStreamBuffer.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.FileOutputStream", FileOutputStream, OutputStream)

FileOutputStream::FileOutputStream(IStream* stream, IEncoding* encoding, LineEnd lineEnd)
:	OutputStream(new FileOutputStreamBuffer(stream, encoding), lineEnd)
{
}

void FileOutputStream::close()
{
	static_cast< FileOutputStreamBuffer* >(getBuffer())->close();
}

}
