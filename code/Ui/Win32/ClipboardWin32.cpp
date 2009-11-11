#include <windows.h>
#include "Ui/Win32/ClipboardWin32.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"

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

bool ClipboardWin32::setObject(Serializable* object)
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
	T_ASSERT (ptr);
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
	T_ASSERT (ptr);
	std::memcpy(ptr, text.c_str(), text.length() * sizeof(wchar_t));
	ptr[text.length()] = 0;
	GlobalUnlock(handle);

	SetClipboardData(CF_UNICODETEXT, handle);
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
		CloseClipboard();
	}
	return contentType;
}

Ref< Serializable > ClipboardWin32::getObject() const
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
	T_ASSERT (ptr);
	MemoryStream ms(&ptr[1], *ptr);
	Ref< Serializable > object = BinarySerializer(&ms).readObject();
	GlobalUnlock(handle);

	CloseClipboard();
	return object;
}

std::wstring ClipboardWin32::getText() const
{
	if (!OpenClipboard(NULL))
		return 0;

	HANDLE handle = GetClipboardData(m_objectFormat);
	if (!handle)
	{
		CloseClipboard();
		return 0;
	}

	wchar_t* ptr = (wchar_t*)GlobalLock(handle);
	T_ASSERT (ptr);
	std::wstring str = ptr;
	GlobalUnlock(handle);

	CloseClipboard();
	return str;
}

	}
}
