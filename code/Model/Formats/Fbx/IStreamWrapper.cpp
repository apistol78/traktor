/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Model/Formats/Fbx/IStreamWrapper.h"

namespace traktor::model
{

//IStreamWrapper::IStreamWrapper()
//:	m_stream(nullptr)
//,	m_state(eEmpty)
//{
//}
//
//IStreamWrapper::~IStreamWrapper()
//{
//	m_stream = nullptr;
//}
//
//FbxStream::EState IStreamWrapper::GetState()
//{
//	return m_state;
//}
//
//bool IStreamWrapper::Open(void* pStreamData)
//{
//	m_stream = static_cast< IStream* >(pStreamData);
//	m_stream->seek(IStream::SeekSet, 0);
//	m_state = eOpen;
//	return true;
//}
//
//bool IStreamWrapper::Close()
//{
//	T_ASSERT(m_stream);
//	m_stream->seek(IStream::SeekSet, 0);
//	m_state = eClosed;
//	return true;
//}
//
//bool IStreamWrapper::Flush()
//{
//	T_ASSERT(m_stream);
//	m_stream->flush();
//	return true;
//}
//
//int IStreamWrapper::Write(const void* /*pData*/, int /*pSize*/)
//{
//	return 0;
//}
//
//int IStreamWrapper::Read(void* pData, int pSize) const
//{
//	T_ASSERT(m_stream);
//	return (int)m_stream->read(pData, pSize);
//}
//
//int IStreamWrapper::GetReaderID() const
//{
//	return -1;
//}
//
//int IStreamWrapper::GetWriterID() const
//{
//	return -1;
//}
//
//void IStreamWrapper::Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
//{
//	T_ASSERT(m_stream);
//	if (pSeekPos == FbxFile::eCurrent)
//		m_stream->seek(IStream::SeekCurrent, int(pOffset));
//	else if (pSeekPos == FbxFile::eBegin)
//		m_stream->seek(IStream::SeekSet, int(pOffset));
//	else if (pSeekPos == FbxFile::eEnd)
//		m_stream->seek(IStream::SeekEnd, int(pOffset));
//}
//
//long IStreamWrapper::GetPosition() const
//{
//	T_ASSERT(m_stream);
//	return (long)m_stream->tell();
//}
//
//void IStreamWrapper::SetPosition(long pPosition)
//{
//	T_ASSERT(m_stream);
//	m_stream->seek(IStream::SeekSet, int(pPosition));
//}
//
//int IStreamWrapper::GetError() const
//{
//	return 0;
//}
//
//void IStreamWrapper::ClearError()
//{
//}

}
