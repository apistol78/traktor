/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionVMImage1_H
#define traktor_flash_ActionVMImage1_H

#include "Core/Containers/AlignedVector.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/IActionVMImage.h"

namespace traktor
{
	namespace flash
	{

class ActionVMImage1 : public IActionVMImage
{
	T_RTTI_CLASS;

public:
	ActionVMImage1();

	ActionVMImage1(const uint8_t* byteCode, uint32_t length);

	void prepare();

	uint16_t addConstData(const ActionValue& cd);

	const ActionValue& getConstData(uint16_t index) const { return m_constData[index]; }

	virtual void execute(ActionFrame* frame) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ActionVM1;

	AlignedVector< uint8_t > m_byteCode;
	AlignedVector< ActionValue > m_constData;
	bool m_prepared;

	void nonConstExecute(ActionFrame* frame);
};

	}
}

#endif	// traktor_flash_ActionVMImage1_H
