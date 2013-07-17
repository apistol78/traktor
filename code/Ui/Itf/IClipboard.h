#ifndef traktor_ui_IClipboard_H
#define traktor_ui_IClipboard_H

#include "Core/Config.h"
#include "Core/Ref.h"
#include "Ui/Enums.h"

namespace traktor
{

class ISerializable;

	namespace drawing
	{

class Image;

	}

	namespace ui
	{

/*! \brief Clipboard interface.
 * \ingroup UI
 */
class IClipboard
{
public:
	virtual void destroy() = 0;
	
	virtual bool setObject(ISerializable* object) = 0;

	virtual bool setText(const std::wstring& text) = 0;

	virtual bool setImage(const drawing::Image* image) = 0;

	virtual ClipboardContentType getContentType() const = 0;

	virtual Ref< ISerializable > getObject() const = 0;

	virtual std::wstring getText() const = 0;

	virtual Ref< const drawing::Image > getImage() const = 0;
};

	}
}

#endif	// traktor_ui_IClipboard_H
