#include <windows.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Ui/Win32/ClipboardWin32.h"

namespace traktor
{
	namespace ui
	{

ClipboardWin32::ClipboardWin32()
{
	m_objectFormat = RegisterClipboardFormat(L"Traktor Clipboard Object");
}

void ClipboardWin32::destroy()
{
	delete this;
}

bool ClipboardWin32::setObject(ISerializable* object)
{
	if (!object)
		return false;

	DynamicMemoryStream dms(false, true);
	if (!BinarySerializer(&dms).writeObject(object))
		return false;

	const std::vector< uint8_t >& buffer = dms.getBuffer();

	if (!OpenClipboard(NULL))
		return false;

	EmptyClipboard();

	HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, sizeof(uint32_t) + buffer.size());
	if (!handle)
	{
		CloseClipboard();
		return false;
	}

	uint32_t* ptr = (uint32_t*)GlobalLock(handle);
	if (!ptr)
	{
		CloseClipboard();
		return false;
	}

	*ptr++ = uint32_t(buffer.size());
	std::memcpy(ptr, &buffer[0], buffer.size());
	GlobalUnlock(handle);

	SetClipboardData(m_objectFormat, handle);
	CloseClipboard();
	return true;
}

bool ClipboardWin32::setText(const std::wstring& text)
{
	if (!OpenClipboard(NULL))
		return false;

	EmptyClipboard();

	HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
	if (!handle)
	{
		CloseClipboard();
		return false;
	}

	wchar_t* ptr = (wchar_t*)GlobalLock(handle);
	if (!ptr)
	{
		CloseClipboard();
		return false;
	}

	std::memcpy(ptr, text.c_str(), text.length() * sizeof(wchar_t));
	ptr[text.length()] = 0;
	GlobalUnlock(handle);

	SetClipboardData(CF_UNICODETEXT, handle);
	CloseClipboard();
	return true;
}

bool ClipboardWin32::setImage(const drawing::Image* image)
{
	if (!OpenClipboard(NULL))
		return false;

	Ref< drawing::Image > dib = image->applyFilter(&drawing::MirrorFilter(false, true));
	dib->convert(drawing::PixelFormat::getA8R8G8B8());

	EmptyClipboard();

	HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dib->getDataSize());
	if (!handle)
	{
		CloseClipboard();
		return false;
	}

	void* ptr = GlobalLock(handle);
	if (!ptr)
	{
		CloseClipboard();
		return false;
	}

	BITMAPINFOHEADER* header = static_cast< BITMAPINFOHEADER* >(ptr);
	std::memset(header, 0, sizeof(BITMAPINFOHEADER));
	header->biSize = sizeof(BITMAPINFOHEADER);
	header->biWidth = dib->getWidth();
	header->biHeight = dib->getHeight();
	header->biPlanes = 1;
	header->biBitCount = 32;
	header->biCompression = BI_RGB;
	header->biSizeImage = 0;
	header->biXPelsPerMeter = 0;
	header->biYPelsPerMeter = 0;
	header->biClrUsed = 0;
	header->biClrImportant = 0;

	std::memcpy(
		header + 1,
		dib->getData(),
		dib->getDataSize()
	);

	GlobalUnlock(handle);

	SetClipboardData(CF_DIB, handle);
	CloseClipboard();
	return true;
}

ClipboardContentType ClipboardWin32::getContentType() const
{
	ClipboardContentType contentType = CtEmpty;
	if (OpenClipboard(NULL))
	{
		if (GetClipboardData(m_objectFormat) != NULL)
			contentType = CtObject;
		else if (GetClipboardData(CF_UNICODETEXT) != NULL)
			contentType = CtText;
		else if (GetClipboardData(CF_DIB) != NULL)
			contentType = CtImage;
		CloseClipboard();
	}
	return contentType;
}

Ref< ISerializable > ClipboardWin32::getObject() const
{
	if (!OpenClipboard(NULL))
		return 0;

	HANDLE handle = GetClipboardData(m_objectFormat);
	if (!handle)
	{
		CloseClipboard();
		return 0;
	}

	uint32_t* ptr = (uint32_t*)GlobalLock(handle);
	if (!ptr)
	{
		CloseClipboard();
		return 0;
	}

	MemoryStream ms(&ptr[1], *ptr);
	Ref< ISerializable > object = BinarySerializer(&ms).readObject();
	GlobalUnlock(handle);

	CloseClipboard();
	return object;
}

std::wstring ClipboardWin32::getText() const
{
	if (!OpenClipboard(NULL))
		return L"";

	HANDLE handle = GetClipboardData(CF_UNICODETEXT);
	if (!handle)
	{
		CloseClipboard();
		return L"";
	}

	wchar_t* ptr = (wchar_t*)GlobalLock(handle);
	if (!ptr)
	{
		CloseClipboard();
		return L"";
	}

	std::wstring str = ptr;
	GlobalUnlock(handle);

	CloseClipboard();
	return str;
}

Ref< const drawing::Image > ClipboardWin32::getImage() const
{
	return 0;
}

	}
}
