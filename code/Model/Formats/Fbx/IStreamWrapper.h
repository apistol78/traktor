/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

//#include <fbxsdk.h>
#include "Core/Ref.h"

namespace traktor
{

class IStream;

}

namespace traktor::model
{

//class IStreamWrapper : public FbxStream
//{
//public:
//	IStreamWrapper();
//
//	virtual ~IStreamWrapper();
//
//	virtual EState GetState() override final;
//
//	virtual bool Open(void* pStreamData) override final;
//
//	virtual bool Close() override final;
//
//	virtual bool Flush() override final;
//
//	virtual int Write(const void* /*pData*/, int /*pSize*/) override final;
//
//	virtual int Read(void* pData, int pSize) const override final;
//
//	virtual int GetReaderID() const override final;
//
//	virtual int GetWriterID() const override final;
//
//	virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos) override final;
//
//	virtual long GetPosition() const override final;
//
//	virtual void SetPosition(long pPosition) override final;
//
//	virtual int GetError() const override final;
//
//	virtual void ClearError() override final;
//
//private:
//	Ref< IStream > m_stream;
//	EState m_state;
//};

}
