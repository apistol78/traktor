#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm2/ActionVM2.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

//struct AbcFile
//{
//	uint16_t minorVersion;
//	uint16_t majorVersion;
//	AbcCPoolInfo* constantPool;
//	uint32_t methodCount;
//	AbcMethodInfo* method;
//	uint32_t metaDataCount;
//	AbcMetaData* metaData;
//	uint32_t classCount;
//	AbcClassInfo* classes;
//	uint32_t scriptCount;
//	AbcScriptInfo* script;
//	uint32_t methodBodyCount;
//	AbcMethodBodyInfo* methodBody;
//};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVM2", ActionVM2, IActionVM)

void ActionVM2::execute(ActionFrame* frame) const
{
	const uint8_t* abcFile = frame->getCode();

	uint16_t minorVersion = *(const uint16_t*)abcFile;
	uint16_t majorVersion = *(const uint16_t*)(abcFile + 2);
}

	}
}
