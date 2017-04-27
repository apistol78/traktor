/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/Avm2/ActionOpcodes.h"

namespace traktor
{
	namespace flash
	{

#define T_AVM2_OP_WIDEN_X(x) L ## x
#define T_AVM2_OP_WIDEN(x) T_AVM2_OP_WIDEN_X(x)

#define T_AVM2_OP(op) \
	{ (op), T_AVM2_OP_WIDEN(#op), 0 }

#define T_AVM2_OP_W(op, width) \
	{ (op), T_AVM2_OP_WIDEN(#op), (width) }

const Avm2OpCodeInfo c_avm2OpCodeInfo[] =
{
	T_AVM2_OP(Avm2OpAdd),
	T_AVM2_OP(Avm2OpAdd_i),
	T_AVM2_OP(Avm2OpAsType),
	T_AVM2_OP(Avm2OpAsTypeLate),
	T_AVM2_OP(Avm2OpBitAnd),
	T_AVM2_OP(Avm2OpBitNot),
	T_AVM2_OP(Avm2OpBitOr),
	T_AVM2_OP(Avm2OpBitXor),
	T_AVM2_OP(Avm2OpCall),
	T_AVM2_OP(Avm2OpCallMethod),
	T_AVM2_OP(Avm2OpCallProperty),
	T_AVM2_OP(Avm2OpCallProplex),
	T_AVM2_OP(Avm2OpCallPropVoid),
	T_AVM2_OP(Avm2OpCallStatic),
	T_AVM2_OP(Avm2OpCallSuper),
	T_AVM2_OP(Avm2OpCallSuperVoid),
	T_AVM2_OP(Avm2OpCheckFilter),
	T_AVM2_OP(Avm2OpCoerce),
	T_AVM2_OP(Avm2OpCoerce_a),
	T_AVM2_OP(Avm2OpCoerce_s),
	T_AVM2_OP(Avm2OpConstruct),
	T_AVM2_OP(Avm2OpConstructProp),
	T_AVM2_OP(Avm2OpConstructSuper),
	T_AVM2_OP(Avm2OpConvert_b),
	T_AVM2_OP(Avm2OpConvert_i),
	T_AVM2_OP(Avm2OpConvert_d),
	T_AVM2_OP(Avm2OpConvert_o),
	T_AVM2_OP(Avm2OpConvert_u),
	T_AVM2_OP(Avm2OpConvert_s),
	T_AVM2_OP(Avm2OpDebug),
	T_AVM2_OP(Avm2OpDebugFile),
	T_AVM2_OP(Avm2OpDebugLine),
	T_AVM2_OP(Avm2OpDecLocal),
	T_AVM2_OP(Avm2OpDecLocal_i),
	T_AVM2_OP(Avm2OpDecrement),
	T_AVM2_OP(Avm2OpDecrement_i),
	T_AVM2_OP(Avm2OpDeleteProperty),
	T_AVM2_OP(Avm2OpDivide),
	T_AVM2_OP(Avm2OpDup),
	T_AVM2_OP(Avm2OpDxns),
	T_AVM2_OP(Avm2OpDxnsLate),
	T_AVM2_OP(Avm2OpEquals),
	T_AVM2_OP(Avm2OpEscXAttr),
	T_AVM2_OP(Avm2OpEscXElem),
	T_AVM2_OP(Avm2OpFindProperty),
	T_AVM2_OP(Avm2OpFindPropStrict),
	T_AVM2_OP(Avm2OpGetDescendants),
	T_AVM2_OP(Avm2OpGetGlobalScope),
	T_AVM2_OP(Avm2OpGetGlobalSlot),
	T_AVM2_OP(Avm2OpGetLex),
	T_AVM2_OP(Avm2OpGetLocal),
	T_AVM2_OP(Avm2OpGetLocal_0),
	T_AVM2_OP(Avm2OpGetLocal_1),
	T_AVM2_OP(Avm2OpGetLocal_2),
	T_AVM2_OP(Avm2OpGetLocal_3),
	T_AVM2_OP(Avm2OpGetProperty),
	T_AVM2_OP(Avm2OpGetScopeObject),
	T_AVM2_OP(Avm2OpGetSlot),
	T_AVM2_OP(Avm2OpGetSuper),
	T_AVM2_OP(Avm2OpGreaterEquals),
	T_AVM2_OP(Avm2OpGreaterThan),
	T_AVM2_OP(Avm2OpHasNext),
	T_AVM2_OP(Avm2OpHasNext2),
	T_AVM2_OP(Avm2OpIfEq),
	T_AVM2_OP(Avm2OpIfFalse),
	T_AVM2_OP(Avm2OpIfGe),
	T_AVM2_OP(Avm2OpIfGt),
	T_AVM2_OP(Avm2OpIfLe),
	T_AVM2_OP(Avm2OpIfLt),
	T_AVM2_OP(Avm2OpIfNge),
	T_AVM2_OP(Avm2OpIfNgt),
	T_AVM2_OP(Avm2OpIfNle),
	T_AVM2_OP(Avm2OpIfNlt),
	T_AVM2_OP(Avm2OpIfNe),
	T_AVM2_OP(Avm2OpIfStrictEq),
	T_AVM2_OP(Avm2OpIfStrictNe),
	T_AVM2_OP(Avm2OpIfTrue),
	T_AVM2_OP(Avm2OpIn),
	T_AVM2_OP(Avm2OpIncLocal),
	T_AVM2_OP(Avm2OpIncLocal_i),
	T_AVM2_OP(Avm2OpIncrement),
	T_AVM2_OP(Avm2OpIncrement_i),
	T_AVM2_OP(Avm2OpInitProperty),
	T_AVM2_OP(Avm2OpInstanceOf),
	T_AVM2_OP(Avm2OpIsType),
	T_AVM2_OP(Avm2OpIsTypeLate),
	T_AVM2_OP(Avm2OpJump),
	T_AVM2_OP(Avm2OpKill),
	T_AVM2_OP(Avm2OpLabel),
	T_AVM2_OP(Avm2OpLessEquals),
	T_AVM2_OP(Avm2OpLessThan),
	T_AVM2_OP(Avm2OpLookupSwitch),
	T_AVM2_OP(Avm2OpLShift),
	T_AVM2_OP(Avm2OpModulo),
	T_AVM2_OP(Avm2OpMultiply),
	T_AVM2_OP(Avm2OpMultiply_i),
	T_AVM2_OP(Avm2OpNegate),
	T_AVM2_OP(Avm2OpNegate_i),
	T_AVM2_OP(Avm2OpNewActivation),
	T_AVM2_OP(Avm2OpNewArray),
	T_AVM2_OP(Avm2OpNewCatch),
	T_AVM2_OP(Avm2OpNewClass),
	T_AVM2_OP(Avm2OpNewFunction),
	T_AVM2_OP(Avm2OpNewObject),
	T_AVM2_OP(Avm2OpNextName),
	T_AVM2_OP(Avm2OpNextValue),
	T_AVM2_OP(Avm2OpNop),
	T_AVM2_OP(Avm2OpNot),
	T_AVM2_OP(Avm2OpPop),
	T_AVM2_OP(Avm2OpPopScope),
	T_AVM2_OP(Avm2OpPushByte),
	T_AVM2_OP(Avm2OpPushDouble),
	T_AVM2_OP(Avm2OpPushFalse),
	T_AVM2_OP(Avm2OpPushInt),
	T_AVM2_OP(Avm2OpPushNamespace),
	T_AVM2_OP(Avm2OpPushNan),
	T_AVM2_OP(Avm2OpPushNull),
	T_AVM2_OP(Avm2OpPushScope),
	T_AVM2_OP(Avm2OpPushShort),
	T_AVM2_OP(Avm2OpPushString),
	T_AVM2_OP(Avm2OpPushTrue),
	T_AVM2_OP(Avm2OpPushUInt),
	T_AVM2_OP(Avm2OpPushUndefined),
	T_AVM2_OP(Avm2OpPushWith),
	T_AVM2_OP(Avm2OpReturnValue),
	T_AVM2_OP(Avm2OpReturnVoid),
	T_AVM2_OP(Avm2OpRShift),
	T_AVM2_OP(Avm2OpSetLocal),
	T_AVM2_OP(Avm2OpSetLocal_0),
	T_AVM2_OP(Avm2OpSetLocal_1),
	T_AVM2_OP(Avm2OpSetLocal_2),
	T_AVM2_OP(Avm2OpSetLocal_3),
	T_AVM2_OP(Avm2OpSetGlobalSlot),
	T_AVM2_OP(Avm2OpSetProperty),
	T_AVM2_OP(Avm2OpSetSlot),
	T_AVM2_OP(Avm2OpSetSuper),
	T_AVM2_OP(Avm2OpStrictEqual),
	T_AVM2_OP(Avm2OpSubtract),
	T_AVM2_OP(Avm2OpSubtract_i),
	T_AVM2_OP(Avm2OpSwap),
	T_AVM2_OP(Avm2OpThrow),
	T_AVM2_OP(Avm2OpTypeOf),
	T_AVM2_OP(Avm2OpURShift),
};

const Avm2OpCodeInfo* findOpCodeInfo(uint8_t op)
{
	for (uint32_t i =  0; i < sizeof_array(c_avm2OpCodeInfo); ++i)
	{
		if (c_avm2OpCodeInfo[i].op == op)
			return &c_avm2OpCodeInfo[i];
	}
	return 0;
}

	}
}
