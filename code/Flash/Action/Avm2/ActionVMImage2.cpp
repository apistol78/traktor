#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm2/ActionOpcodes.h"
#include "Flash/Action/Avm2/ActionVMImage2.h"

#define VM_TRACE_ENABLE 0

#if VM_TRACE_ENABLE
#	define T_WIDEN_X(x) L ## x
#	define T_WIDEN(x) T_WIDEN_X(x)
#	define VM_BEGIN(op) \
	case op : \
		{ \
		log::debug << T_WIDEN( #op ) << L" (stack " << stack.depth() << L")" << Endl << IncreaseIndent;
#	define VM_END() \
	log::debug << DecreaseIndent; \
		} \
		break;
#	define VM_NOT_IMPLEMENTED \
	log::error << L"Opcode not implemented" << Endl; \
	T_BREAKPOINT;
#	define VM_LOG(x) \
	T_DEBUG(x);
#else
#	define VM_BEGIN(op) \
	case op : \
		{
#	define VM_END() \
		} \
		break;
#	define VM_NOT_IMPLEMENTED
#	define VM_LOG(x)
#endif

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ActionVMImage2", 0, ActionVMImage2, IActionVMImage)

void ActionVMImage2::execute(ActionFrame* frame) const
{
	const uint8_t T_UNALIGNED * abcFile = 0; //frame->getCode();

	uint16_t minorVersion = *(const uint16_t*)abcFile;
	uint16_t majorVersion = *(const uint16_t*)(abcFile + 2);

	const uint8_t T_UNALIGNED * pc = abcFile;
	T_ASSERT (pc);

	ActionValueStack& stack = frame->getStack();

	for (;;)
	{
		uint8_t op = *pc++;
		switch (op)
		{
		VM_BEGIN(Avm2OpAdd)
			ActionValue value2 = stack.pop();
			ActionValue value1 = stack.pop();
		VM_END()

		VM_BEGIN(Avm2OpAdd_i)
			ActionValue value2 = stack.pop();
			ActionValue value1 = stack.pop();
			avm_number_t result = value1.getNumber() + value2.getNumber();
			stack.push(ActionValue(result));
		VM_END()

		VM_BEGIN(Avm2OpAsType)
		VM_END()

		VM_BEGIN(Avm2OpAsTypeLate)
		VM_END()

		VM_BEGIN(Avm2OpBitAnd)
		VM_END()

		VM_BEGIN(Avm2OpBitNot)
		VM_END()

		VM_BEGIN(Avm2OpBitOr)
		VM_END()

		VM_BEGIN(Avm2OpBitXor)
		VM_END()

		VM_BEGIN(Avm2OpCall)
		VM_END()

		VM_BEGIN(Avm2OpCallMethod)
		VM_END()

		VM_BEGIN(Avm2OpCallProperty)
		VM_END()

		VM_BEGIN(Avm2OpCallProplex)
		VM_END()

		VM_BEGIN(Avm2OpCallPropVoid)
		VM_END()

		VM_BEGIN(Avm2OpCallStatic)
		VM_END()

		VM_BEGIN(Avm2OpCallSuper)
		VM_END()

		VM_BEGIN(Avm2OpCallSuperVoid)
		VM_END()

		VM_BEGIN(Avm2OpCheckFilter)
		VM_END()

		VM_BEGIN(Avm2OpCoerce)
		VM_END()

		VM_BEGIN(Avm2OpCoerce_a)
		VM_END()

		VM_BEGIN(Avm2OpCoerce_s)
		VM_END()

		VM_BEGIN(Avm2OpConstruct)
		VM_END()

		VM_BEGIN(Avm2OpConstructProp)
		VM_END()

		VM_BEGIN(Avm2OpConstructSuper)
		VM_END()

		VM_BEGIN(Avm2OpConvert_b)
		VM_END()

		VM_BEGIN(Avm2OpConvert_i)
		VM_END()

		VM_BEGIN(Avm2OpConvert_d)
		VM_END()

		VM_BEGIN(Avm2OpConvert_o)
		VM_END()

		VM_BEGIN(Avm2OpConvert_u)
		VM_END()

		VM_BEGIN(Avm2OpConvert_s)
		VM_END()

		VM_BEGIN(Avm2OpDebug)
		VM_END()

		VM_BEGIN(Avm2OpDebugFile)
		VM_END()

		VM_BEGIN(Avm2OpDebugLine)
		VM_END()

		VM_BEGIN(Avm2OpDecLocal)
		VM_END()

		VM_BEGIN(Avm2OpDecLocal_i)
		VM_END()

		VM_BEGIN(Avm2OpDecrement)
		VM_END()

		VM_BEGIN(Avm2OpDecrement_i)
		VM_END()

		VM_BEGIN(Avm2OpDeleteProperty)
		VM_END()

		VM_BEGIN(Avm2OpDivide)
		VM_END()

		VM_BEGIN(Avm2OpDup)
		VM_END()

		VM_BEGIN(Avm2OpDxns)
		VM_END()

		VM_BEGIN(Avm2OpDxnsLate)
		VM_END()

		VM_BEGIN(Avm2OpEquals)
		VM_END()

		VM_BEGIN(Avm2OpEscXAttr)
		VM_END()

		VM_BEGIN(Avm2OpEscXElem)
		VM_END()

		VM_BEGIN(Avm2OpFindProperty)
		VM_END()

		VM_BEGIN(Avm2OpFindPropStrict)
		VM_END()

		VM_BEGIN(Avm2OpGetDescendants)
		VM_END()

		VM_BEGIN(Avm2OpGetGlobalScope)
		VM_END()

		VM_BEGIN(Avm2OpGetGlobalSlot)
		VM_END()

		VM_BEGIN(Avm2OpGetLex)
		VM_END()

		VM_BEGIN(Avm2OpGetLocal)
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_0)
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_1)
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_2)
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_3)
		VM_END()

		VM_BEGIN(Avm2OpGetProperty)
		VM_END()

		VM_BEGIN(Avm2OpGetScopeObject)
		VM_END()

		VM_BEGIN(Avm2OpGetSlot)
		VM_END()

		VM_BEGIN(Avm2OpGetSuper)
		VM_END()

		VM_BEGIN(Avm2OpGreaterEquals)
		VM_END()

		VM_BEGIN(Avm2OpGreaterThan)
		VM_END()

		VM_BEGIN(Avm2OpHasNext)
		VM_END()

		VM_BEGIN(Avm2OpHasNext2)
		VM_END()

		VM_BEGIN(Avm2OpIfEq)
		VM_END()

		VM_BEGIN(Avm2OpIfFalse)
		VM_END()

		VM_BEGIN(Avm2OpIfGe)
		VM_END()

		VM_BEGIN(Avm2OpIfGt)
		VM_END()

		VM_BEGIN(Avm2OpIfLe)
		VM_END()

		VM_BEGIN(Avm2OpIfLt)
		VM_END()

		VM_BEGIN(Avm2OpIfNge)
		VM_END()

		VM_BEGIN(Avm2OpIfNgt)
		VM_END()

		VM_BEGIN(Avm2OpIfNle)
		VM_END()

		VM_BEGIN(Avm2OpIfNlt)
		VM_END()

		VM_BEGIN(Avm2OpIfNe)
		VM_END()

		VM_BEGIN(Avm2OpIfStrictEq)
		VM_END()

		VM_BEGIN(Avm2OpIfStrictNe)
		VM_END()

		VM_BEGIN(Avm2OpIfTrue)
		VM_END()

		VM_BEGIN(Avm2OpIn)
		VM_END()

		VM_BEGIN(Avm2OpIncLocal)
		VM_END()

		VM_BEGIN(Avm2OpIncLocal_i)
		VM_END()

		VM_BEGIN(Avm2OpIncrement)
		VM_END()

		VM_BEGIN(Avm2OpIncrement_i)
		VM_END()

		VM_BEGIN(Avm2OpInitProperty)
		VM_END()

		VM_BEGIN(Avm2OpInstanceOf)
		VM_END()

		VM_BEGIN(Avm2OpIsType)
		VM_END()

		VM_BEGIN(Avm2OpIsTypeLate)
		VM_END()

		VM_BEGIN(Avm2OpJump)
		VM_END()

		VM_BEGIN(Avm2OpKill)
		VM_END()

		VM_BEGIN(Avm2OpLabel)
		VM_END()

		VM_BEGIN(Avm2OpLessEquals)
		VM_END()

		VM_BEGIN(Avm2OpLessThan)
		VM_END()

		VM_BEGIN(Avm2OpLookupSwitch)
		VM_END()

		VM_BEGIN(Avm2OpLShift)
		VM_END()

		VM_BEGIN(Avm2OpModulo)
		VM_END()

		VM_BEGIN(Avm2OpMultiply)
		VM_END()

		VM_BEGIN(Avm2OpMultiply_i)
		VM_END()

		VM_BEGIN(Avm2OpNegate)
		VM_END()

		VM_BEGIN(Avm2OpNegate_i)
		VM_END()

		VM_BEGIN(Avm2OpNewActivation)
		VM_END()

		VM_BEGIN(Avm2OpNewArray)
		VM_END()

		VM_BEGIN(Avm2OpNewCatch)
		VM_END()

		VM_BEGIN(Avm2OpNewClass)
		VM_END()

		VM_BEGIN(Avm2OpNewFunction)
		VM_END()

		VM_BEGIN(Avm2OpNewObject)
		VM_END()

		VM_BEGIN(Avm2OpNextName)
		VM_END()

		VM_BEGIN(Avm2OpNextValue)
		VM_END()

		VM_BEGIN(Avm2OpNop)
		VM_END()

		VM_BEGIN(Avm2OpNot)
		VM_END()

		VM_BEGIN(Avm2OpPop)
		VM_END()

		VM_BEGIN(Avm2OpPopScope)
		VM_END()

		VM_BEGIN(Avm2OpPushByte)
		VM_END()

		VM_BEGIN(Avm2OpPushDouble)
		VM_END()

		VM_BEGIN(Avm2OpPushFalse)
		VM_END()

		VM_BEGIN(Avm2OpPushInt)
		VM_END()

		VM_BEGIN(Avm2OpPushNamespace)
		VM_END()

		VM_BEGIN(Avm2OpPushNan)
		VM_END()

		VM_BEGIN(Avm2OpPushNull)
		VM_END()

		VM_BEGIN(Avm2OpPushScope)
		VM_END()

		VM_BEGIN(Avm2OpPushShort)
		VM_END()

		VM_BEGIN(Avm2OpPushString)
		VM_END()

		VM_BEGIN(Avm2OpPushTrue)
		VM_END()

		VM_BEGIN(Avm2OpPushUInt)
		VM_END()

		VM_BEGIN(Avm2OpPushUndefined)
		VM_END()

		VM_BEGIN(Avm2OpPushWith)
		VM_END()

		VM_BEGIN(Avm2OpReturnValue)
		VM_END()

		VM_BEGIN(Avm2OpReturnVoid)
		VM_END()

		VM_BEGIN(Avm2OpRShift)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_0)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_1)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_2)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_3)
		VM_END()

		VM_BEGIN(Avm2OpSetGlobalSlot)
		VM_END()

		VM_BEGIN(Avm2OpSetProperty)
		VM_END()

		VM_BEGIN(Avm2OpSetSlot)
		VM_END()

		VM_BEGIN(Avm2OpSetSuper)
		VM_END()

		VM_BEGIN(Avm2OpStrictEqual)
		VM_END()

		VM_BEGIN(Avm2OpSubtract)
		VM_END()

		VM_BEGIN(Avm2OpSubtract_i)
		VM_END()

		VM_BEGIN(Avm2OpSwap)
		VM_END()

		VM_BEGIN(Avm2OpThrow)
		VM_END()

		VM_BEGIN(Avm2OpTypeOf)
		VM_END()

		VM_BEGIN(Avm2OpURShift)
		VM_END()
		}
	}
}

void ActionVMImage2::serialize(ISerializer& s)
{
}

	}
}
