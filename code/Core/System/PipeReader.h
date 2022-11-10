/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! Process pipe line reader.
 * \ingroup Core
 */
class T_DLLCLASS PipeReader : public Object
{
	T_RTTI_CLASS;

public:
	enum Result
	{
		RtOk,
		RtEmpty,
		RtEnd
	};

	explicit PipeReader(IStream* stream);

	virtual ~PipeReader();

	Result readLine(std::wstring& outLine);

private:
	Ref< IStream > m_stream;
	AlignedVector< char > m_acc;
	std::list< std::wstring > m_lines;
};

}

