#pragma once

#include "Ui/Itf/IClipboard.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UI
 */
class ClipboardX11 : public IClipboard
{
public:
	ClipboardX11();

	virtual void destroy() override final;

	virtual bool setObject(ISerializable* object) override final;

	virtual bool setText(const std::wstring& text) override final;

	virtual bool setImage(const drawing::Image* image) override final;

	virtual ClipboardContentType getContentType() const override final;

	virtual Ref< ISerializable > getObject() const override final;

	virtual std::wstring getText() const override final;

	virtual Ref< const drawing::Image > getImage() const override final;

private:
	ClipboardContentType m_type;
	Ref< ISerializable > m_object;
	std::wstring m_text;
	Ref< const drawing::Image > m_image;
};

	}
}

