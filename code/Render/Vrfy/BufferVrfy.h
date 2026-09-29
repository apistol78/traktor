/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Thread/SpinLock.h"
#include "Render/Buffer.h"
#include "Render/Vrfy/BufferViewVrfy.h"

#include <string>

namespace traktor::render
{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class BufferVrfy : public Buffer
{
	T_RTTI_CLASS;

public:
	explicit BufferVrfy(ResourceTracker* resourceTracker, Buffer* buffer, uint32_t usage, uint32_t bufferSize, const wchar_t* const tag);

	virtual ~BufferVrfy();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

	virtual void nextFrame() override final;

	Buffer* getWrappedBuffer() const { return m_buffer; }

protected:
	Ref< ResourceTracker > m_resourceTracker;
	Ref< Buffer > m_buffer;
	std::wstring m_tag;
	uint32_t m_usage = 0;
	mutable RefArray< BufferViewVrfy > m_bufferViews; //!< One per view of the wrapped buffer, thus views handed out never change.
	mutable SpinLock m_bufferViewsLock;
	bool m_locked = false;
	uint8_t* m_device = nullptr;
	uint8_t* m_shadow = nullptr;
	void* m_callstack[8] = { nullptr };

	void verifyGuard() const;

	void verifyUntouched() const;
};

}
