#import "Ui/Cocoa/NSComboBoxDataSource.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

@implementation NSComboBoxDataSource

- (void) setCallback: (IComboBoxDataCallback*)callback
{
	m_callback = callback;
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString
{
	int index = m_callback->comboBox_indexOf(traktor::ui::fromNSString(aString));
	return index >= 0 ? index : NSNotFound;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index
{
	return traktor::ui::makeNSString(m_callback->comboBox_valueAt(index));
}

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
	return m_callback->comboBox_numberOfItems();
}

@end
