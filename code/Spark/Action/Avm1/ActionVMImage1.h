#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Spark/Action/ActionValue.h"
#include "Spark/Action/IActionVMImage.h"

namespace traktor
{
	namespace spark
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

	virtual void execute(ActionFrame* frame) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ActionVM1;

	AlignedVector< uint8_t > m_byteCode;
	AlignedVector< ActionValue > m_constData;
	bool m_prepared;

	void nonConstExecute(ActionFrame* frame);
};

	}
}

