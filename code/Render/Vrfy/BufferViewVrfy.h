/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/IBufferView.h"

namespace traktor::render
{

class BufferVrfy;

/*!
 * \ingroup Vrfy
 */
class BufferViewVrfy : public IBufferView
{
	T_RTTI_CLASS;

public:
	explicit BufferViewVrfy(const BufferVrfy* buffer);

	const BufferVrfy* getBuffer() const { return m_buffer; }

	const IBufferView* getWrappedBufferView() const { return m_wrappedBufferView; }

private:
	friend class BufferVrfy;

	const BufferVrfy* m_buffer = nullptr;
	const IBufferView* m_wrappedBufferView = nullptr;
};

}
