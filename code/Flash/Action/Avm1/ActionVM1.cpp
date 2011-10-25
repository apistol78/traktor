#include <cstring>
#include <cmath>
#include "Core/Io/BitReader.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionOpcodes.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionVM1.h"
#include "Flash/Action/Avm1/ActionVMImage1.h"
#include "Flash/Action/Avm1/ActionVMTrace1.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVM1", ActionVM1, IActionVM)

ActionVM1::ActionVM1()
{
	m_timer.start();

#if defined(_DEBUG)
	m_trace = new ActionVMTrace1();
#endif
}

Ref< const IActionVMImage > ActionVM1::load(BitReader& br) const
{
	Ref< ActionVMImage1 > image = new ActionVMImage1();

	image->m_byteCode.reserve(4096);
	for (;;)
	{
		uint8_t opcode = br.readUInt8();
		image->m_byteCode.push_back(opcode);

		if (opcode & 0x80)
		{
			uint16_t length = br.readUInt16();
			image->m_byteCode.push_back(reinterpret_cast< uint8_t* >(&length)[0]);
			image->m_byteCode.push_back(reinterpret_cast< uint8_t* >(&length)[1]);
			for (uint16_t i = 0; i < length; ++i)
			{
				uint8_t data = br.readUInt8();
				image->m_byteCode.push_back(data);
			}
		}

		if (opcode == AopEnd)
			break;
	}

	image->prepare();

	return image;
}

void ActionVM1::execute(ActionFrame* frame) const
{
	const ActionVMImage1* image = static_cast< const ActionVMImage1* >(frame->getImage());
	T_ASSERT (image);

	image->execute(frame, m_timer, m_trace);
}

	}
}
