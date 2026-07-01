/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <poll.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Ui/Wl/ClipboardWl.h"
#include "Ui/Wl/ContextWl.h"

namespace traktor::ui
{

namespace
{

const char* c_mimeObject = "application/x-traktor-object";
const char* c_mimeTextUtf8 = "text/plain;charset=utf-8";
const char* c_mimeTextPlain = "text/plain";

}

static const wl_data_source_listener s_dataSourceListener = {
	ClipboardWl::dataSourceTarget,
	ClipboardWl::dataSourceSend,
	ClipboardWl::dataSourceCancelled
};

ClipboardWl::ClipboardWl(ContextWl* context)
:	m_context(context)
{
}

void ClipboardWl::destroy()
{
	if (m_dataSource)
	{
		wl_data_source_destroy(m_dataSource);
		m_dataSource = nullptr;
	}
	delete this;
}

bool ClipboardWl::setObject(ISerializable* object)
{
	if (!object || !m_context->getDataDeviceManager())
		return false;

	// Serialize to buffer.
	DynamicMemoryStream dms(false, true);
	if (!BinarySerializer(&dms).writeObject(object))
		return false;

	const auto& buf = dms.getBuffer();
	m_objectBuffer = buf;
	m_localObject = object;
	m_localType = ClipboardContent::Object;
	m_textBuffer.clear();

	return createDataSource(c_mimeObject);
}

bool ClipboardWl::setText(const std::wstring& text)
{
	if (!m_context->getDataDeviceManager())
		return false;

	m_textBuffer = wstombs(text);
	m_localType = ClipboardContent::Text;
	m_localObject = nullptr;
	m_objectBuffer.clear();

	return createDataSource(c_mimeTextUtf8);
}

bool ClipboardWl::setImage(const drawing::Image* image)
{
	// In-memory only, matching Win32 which doesn't implement getImage.
	m_localType = ClipboardContent::Image;
	m_localImage = image;
	return true;
}

ClipboardContent ClipboardWl::getContentType() const
{
	// If we own the clipboard, return our local type.
	if (m_dataSource)
		return m_localType;

	// Check the selection offer's MIME types.
	const auto& mimeTypes = m_context->getSelectionMimeTypes();
	for (const auto& mt : mimeTypes)
	{
		if (mt == c_mimeObject)
			return ClipboardContent::Object;
	}
	for (const auto& mt : mimeTypes)
	{
		if (mt == c_mimeTextUtf8 || mt == c_mimeTextPlain || mt == "UTF8_STRING")
			return ClipboardContent::Text;
	}
	return ClipboardContent::Empty;
}

Ref< ISerializable > ClipboardWl::getObject() const
{
	// If we own the clipboard, return local copy.
	if (m_dataSource && m_localType == ClipboardContent::Object)
		return DeepClone(m_localObject).create();

	// Read from the Wayland selection offer.
	const auto data = readFromOffer(c_mimeObject);
	if (data.empty())
		return nullptr;

	MemoryStream ms(data.c_ptr(), (int64_t)data.size());
	return BinarySerializer(&ms).readObject();
}

std::wstring ClipboardWl::getText() const
{
	// If we own the clipboard, return local copy.
	if (m_dataSource && m_localType == ClipboardContent::Text)
		return mbstows(m_textBuffer);

	// Try UTF-8 first, then plain text.
	auto data = readFromOffer(c_mimeTextUtf8);
	if (data.empty())
		data = readFromOffer(c_mimeTextPlain);
	if (data.empty())
		data = readFromOffer("UTF8_STRING");
	if (data.empty())
		return L"";

	return mbstows(std::string(data.begin(), data.end()));
}

Ref< const drawing::Image > ClipboardWl::getImage() const
{
	if (m_dataSource && m_localType == ClipboardContent::Image)
		return m_localImage;
	return nullptr;
}

bool ClipboardWl::createDataSource(const char* mimeType)
{
	wl_data_device_manager* mgr = m_context->getDataDeviceManager();
	wl_data_device* device = m_context->getDataDevice();
	if (!mgr || !device)
		return false;

	// Destroy previous source.
	if (m_dataSource)
	{
		wl_data_source_destroy(m_dataSource);
		m_dataSource = nullptr;
	}

	m_dataSource = wl_data_device_manager_create_data_source(mgr);
	wl_data_source_add_listener(m_dataSource, &s_dataSourceListener, this);
	wl_data_source_offer(m_dataSource, mimeType);

	// Also offer plain text alias when setting text.
	if (std::strcmp(mimeType, c_mimeTextUtf8) == 0)
	{
		wl_data_source_offer(m_dataSource, c_mimeTextPlain);
		wl_data_source_offer(m_dataSource, "UTF8_STRING");
	}

	wl_data_device_set_selection(device, m_dataSource, m_context->getInputSerial());
	wl_display_flush(m_context->getDisplay());
	return true;
}

AlignedVector< uint8_t > ClipboardWl::readFromOffer(const char* mimeType) const
{
	wl_data_offer* offer = m_context->getSelectionOffer();
	if (!offer)
		return {};

	// Verify the offer has the requested MIME type.
	const auto& mimeTypes = m_context->getSelectionMimeTypes();
	bool found = false;
	for (const auto& mt : mimeTypes)
	{
		if (mt == mimeType)
		{
			found = true;
			break;
		}
	}
	if (!found)
		return {};

	// Create a pipe for data transfer.
	int fds[2];
	if (pipe(fds) != 0)
		return {};

	wl_data_offer_receive(offer, mimeType, fds[1]);
	wl_display_flush(m_context->getDisplay());
	close(fds[1]);

	// Read all data from the pipe.
	AlignedVector< uint8_t > result;
	uint8_t buf[4096];
	for (;;)
	{
		// Poll with timeout to avoid hanging indefinitely.
		struct pollfd pfd = { fds[0], POLLIN, 0 };
		const int pr = poll(&pfd, 1, 1000);
		if (pr <= 0)
			break;

		const ssize_t n = read(fds[0], buf, sizeof(buf));
		if (n <= 0)
			break;

		result.insert(result.end(), buf, buf + n);
	}

	close(fds[0]);
	return result;
}

// Data source callbacks
void ClipboardWl::dataSourceTarget(void* data, wl_data_source* source, const char* mimeType)
{
}

void ClipboardWl::dataSourceSend(void* data, wl_data_source* source, const char* mimeType, int32_t fd)
{
	ClipboardWl* self = static_cast< ClipboardWl* >(data);

	const uint8_t* ptr = nullptr;
	size_t size = 0;

	if (std::strcmp(mimeType, c_mimeObject) == 0)
	{
		ptr = self->m_objectBuffer.c_ptr();
		size = self->m_objectBuffer.size();
	}
	else if (
		std::strcmp(mimeType, c_mimeTextUtf8) == 0 ||
		std::strcmp(mimeType, c_mimeTextPlain) == 0 ||
		std::strcmp(mimeType, "UTF8_STRING") == 0)
	{
		ptr = reinterpret_cast< const uint8_t* >(self->m_textBuffer.c_str());
		size = self->m_textBuffer.size();
	}

	if (ptr && size > 0)
	{
		size_t offset = 0;
		while (offset < size)
		{
			const ssize_t n = write(fd, ptr + offset, size - offset);
			if (n <= 0)
				break;
			offset += n;
		}
	}

	close(fd);
}

void ClipboardWl::dataSourceCancelled(void* data, wl_data_source* source)
{
	ClipboardWl* self = static_cast< ClipboardWl* >(data);
	if (self->m_dataSource == source)
	{
		wl_data_source_destroy(self->m_dataSource);
		self->m_dataSource = nullptr;
		self->m_localType = ClipboardContent::Empty;
		self->m_localObject = nullptr;
		self->m_objectBuffer.clear();
		self->m_textBuffer.clear();
		self->m_localImage = nullptr;
	}
}

}
