/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ImageStructBuffer;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageStructBufferData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< const ImageStructBuffer > createInstance(int32_t instance) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ImageGraphPipeline;

	std::wstring m_id;
	std::wstring m_persistentHandle;
	uint32_t m_bufferSize = 0;
};

}
