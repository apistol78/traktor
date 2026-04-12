/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <wayland-client.h>
#include "Core/Containers/AlignedVector.h"
#include "Ui/Itf/IClipboard.h"

namespace traktor::ui
{

class ContextWl;

/*!
 * \ingroup UI
 */
class ClipboardWl : public IClipboard
{
public:
	explicit ClipboardWl(ContextWl* context);

	virtual void destroy() override final;

	virtual bool setObject(ISerializable* object) override final;

	virtual bool setText(const std::wstring& text) override final;

	virtual bool setImage(const drawing::Image* image) override final;

	virtual ClipboardContent getContentType() const override final;

	virtual Ref< ISerializable > getObject() const override final;

	virtual std::wstring getText() const override final;

	virtual Ref< const drawing::Image > getImage() const override final;

	// Data source listener callbacks (must be public for listener struct init).
	static void dataSourceTarget(void* data, wl_data_source* source, const char* mimeType);
	static void dataSourceSend(void* data, wl_data_source* source, const char* mimeType, int32_t fd);
	static void dataSourceCancelled(void* data, wl_data_source* source);

private:
	ContextWl* m_context;
	wl_data_source* m_dataSource = nullptr;
	ClipboardContent m_localType = ClipboardContent::Empty;
	Ref< ISerializable > m_localObject;
	AlignedVector< uint8_t > m_objectBuffer;
	std::string m_textBuffer;
	Ref< const drawing::Image > m_localImage;

	bool createDataSource(const char* mimeType);

	AlignedVector< uint8_t > readFromOffer(const char* mimeType) const;
};

}
