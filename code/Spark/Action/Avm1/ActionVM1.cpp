#include "Core/Io/BitReader.h"
#include "Spark/SwfReader.h"
#include "Spark/Action/Avm1/ActionOpcodes.h"
#include "Spark/Action/Avm1/ActionVM1.h"
#include "Spark/Action/Avm1/ActionVMImage1.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionVM1", ActionVM1, IActionVM)

Ref< const IActionVMImage > ActionVM1::load(SwfReader& swf) const
{
	Ref< ActionVMImage1 > image = new ActionVMImage1();
	BitReader& br = swf.getBitReader();

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

	return image;
}

	}
}
