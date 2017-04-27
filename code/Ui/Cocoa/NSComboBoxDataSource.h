/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/

#import <Cocoa/Cocoa.h>

#include <string>

struct IComboBoxDataCallback
{
	virtual int comboBox_indexOf(const std::wstring& value) const = 0;
	
	virtual std::wstring comboBox_valueAt(int index) const = 0;
	
	virtual int comboBox_numberOfItems() const = 0;
};

@interface NSComboBoxDataSource : NSObject < NSComboBoxDataSource >
{
	IComboBoxDataCallback* m_callback;
}

- (id) init;

- (void) setCallback: (IComboBoxDataCallback*)callback;

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString;

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index;

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox;

@end
