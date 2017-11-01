#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Ui/Cocoa/ClipboardCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{
		namespace 
		{

NSString* s_objectDataPboardType = @"Traktor Pasteboard Object";

		}

void ClipboardCocoa::destroy()
{
	delete this;
}

bool ClipboardCocoa::setObject(ISerializable* object)
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];

	if (!object)
		return false;

	DynamicMemoryStream dms(false, true);
	if (!BinarySerializer(&dms).writeObject(object))
		return false;

	const AlignedVector< uint8_t >& buffer = dms.getBuffer();
	if (buffer.empty())
		return false;
	
	NSData* data = [NSData dataWithBytes: &buffer[0] length: buffer.size()];
	
	NSArray* pbTypes = [NSArray arrayWithObjects:s_objectDataPboardType, nil];
	[pb declareTypes:pbTypes owner:nil];
	[pb setData: data forType:s_objectDataPboardType];
	
	return true;
}

bool ClipboardCocoa::setText(const std::wstring& text)
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	
	[pb declareTypes: [NSArray arrayWithObjects: NSStringPboardType, nil] owner: nil];
	[pb setString: makeNSString(text) forType: NSStringPboardType];
	
	return true;
}

bool ClipboardCocoa::setImage(const drawing::Image *image)
{
	return false;
}

ClipboardContentType ClipboardCocoa::getContentType() const
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];

	if ([pb dataForType: s_objectDataPboardType])
		return CtObject;

	return CtEmpty;
}

Ref< ISerializable > ClipboardCocoa::getObject() const
{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];

	NSData* data = [pb dataForType: s_objectDataPboardType];
	if (!data)
		return 0;
		
	uint32_t length = [data length];
	const void* ptr = [data bytes];
	
	if (!ptr || !length)
		return 0;
		
	MemoryStream ms(ptr, length);
	Ref< ISerializable > object = BinarySerializer(&ms).readObject();

	return object;
}

std::wstring ClipboardCocoa::getText() const
{
	return L"";
}

Ref< const drawing::Image > ClipboardCocoa::getImage() const
{
	return 0;
}

	}
}
