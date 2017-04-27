/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/Avm2/ActionOpcodes.h"
#include "Flash/Action/Avm2/ActionVMImage2.h"

#define VM_TRACE_ENABLE 1

#if VM_TRACE_ENABLE
#	define T_WIDEN_X(x) L ## x
#	define T_WIDEN(x) T_WIDEN_X(x)
#	define VM_BEGIN(op) \
	case op : \
		{ \
		log::debug << T_WIDEN( #op ) << L" (scope stack " << int32_t(scopeStack.size()) << L", op stack " << operationStack.depth() << L")" << Endl << IncreaseIndent;
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
		namespace
		{

uint32_t decodeU30(const uint8_t T_UNALIGNED *& pc)
{
	uint32_t out = 0;
	for (uint32_t i = 0; i < 5; ++i)
	{
		uint8_t v = *pc++;
		out |= (v & 0x7f) << (i * 7);
		if ((v & 0x80) == 0x00)
			break;
	}
	return out;
}

bool getMemberOrProperty(ActionFrame* frame, ActionObject* self, int32_t variableId, ActionValue& outValue)
{
	ActionValueStack& stack = frame->getStack();
	Ref< ActionFunction > propertyGet;

	if (self->getPropertyGet(variableId, propertyGet))
	{
		stack.push(ActionValue(0.0f));
		outValue = propertyGet->call(frame, self);
		return true;
	}

	return self->getMember(variableId, outValue);
}

bool getStrictProperty(ActionFrame* frame, ActionObject* self, const AbcFile& abcFile, const MultinameInfo& mn, ActionValue& outValue)
{
	const NamespaceInfo& ns = abcFile.cpool.namespaces[mn.data.qname.ns];

	Ref< ActionObject > it = self;

	if (!abcFile.cpool.strings[ns.name].empty())
	{
		StringSplit< std::string > ss(abcFile.cpool.strings[ns.name], ".");
		for (StringSplit< std::string >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			ActionValue value;
			if (!it->getMember(*i, value))
				return false;
			if (!value.isObject())
				return false;
			it = value.getObject();
		}
	}

	T_FATAL_ASSERT (it != 0);

	return it->getMember(abcFile.cpool.strings[mn.data.qname.name], outValue);
}

void setStrictProperty(ActionFrame* frame, ActionObject* self, const AbcFile& abcFile, const MultinameInfo& mn, const ActionValue& value)
{
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ActionVMImage2", 0, ActionVMImage2, IActionVMImage)

void ActionVMImage2::execute(ActionFrame* frame) const
{
	ActionContext* context = frame->getContext();
	T_ASSERT (context);

	// Last script is the first to be executed of ABC.
	const ScriptInfo& script = m_abcFile.scripts.back();

	const MethodInfo& method = m_abcFile.methods[script.init];
	log::info << L"Calling \"" << mbstows(m_abcFile.cpool.strings[method.name]) << L"\"..." << Endl;

	// Find initialization method body.
	const MethodBodyInfo* methodBody = 0;
	for (uint32_t i = 0; i < m_abcFile.methodBodies.size(); ++i)
	{
		if (m_abcFile.methodBodies[i].method == script.init)
		{
			methodBody = &m_abcFile.methodBodies[i];
			break;
		}
	}
	if (!methodBody)
		return;

	ActionValueStack& operationStack = frame->getStack();
	RefArray< ActionObject > scopeStack;

	// Interprete bytecode.
	const uint8_t T_UNALIGNED * pc = methodBody->code.c_ptr();
	for (;;)
	{
		uint8_t op = *pc++;
		switch (op)
		{
		VM_BEGIN(Avm2OpAdd)
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			operationStack.push(value1 + value2);
		VM_END()

		VM_BEGIN(Avm2OpAdd_i)
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			int32_t result = value1.getInteger() + value2.getInteger();
			operationStack.push(ActionValue(result));
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
			uint32_t index = decodeU30(pc);

			const MultinameInfo& mn = m_abcFile.cpool.multinames[index];
			T_FATAL_ASSERT (mn.kind == Mnik_CONSTANT_QName);

			ActionValue value;
			for (size_t i = 0; i < scopeStack.size(); ++i)
			{
				if (getStrictProperty(frame, scopeStack[i], m_abcFile, mn, value))
					break;
			}

			operationStack.push(value);

		VM_END()

		VM_BEGIN(Avm2OpGetLocal)
			uint32_t index = decodeU30(pc);
			operationStack.push(frame->getRegister(index));
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_0)
			operationStack.push(frame->getRegister(0));
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_1)
			operationStack.push(frame->getRegister(1));
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_2)
			operationStack.push(frame->getRegister(2));
		VM_END()

		VM_BEGIN(Avm2OpGetLocal_3)
			operationStack.push(frame->getRegister(3));
		VM_END()

		VM_BEGIN(Avm2OpGetProperty)
		VM_END()

		VM_BEGIN(Avm2OpGetScopeObject)
			int32_t offset = *pc++;
			operationStack.push(ActionValue(scopeStack.front() + offset));
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
			uint32_t index = decodeU30(pc);

			const MultinameInfo& mn = m_abcFile.cpool.multinames[index];
			

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
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			operationStack.push(value1 * value2);
		VM_END()

		VM_BEGIN(Avm2OpMultiply_i)
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			int32_t result = value1.getInteger() * value2.getInteger();
			operationStack.push(ActionValue(result));
		VM_END()

		VM_BEGIN(Avm2OpNegate)
			ActionValue& top = operationStack.top();
			if (top.isNumeric())
				top = ActionValue(-top.getFloat());
			else
				top = ActionValue();
		VM_END()

		VM_BEGIN(Avm2OpNegate_i)
			ActionValue& top = operationStack.top();
			if (top.isNumeric())
				top = ActionValue(-top.getInteger());
			else
				top = ActionValue();
		VM_END()

		VM_BEGIN(Avm2OpNewActivation)
		VM_END()

		VM_BEGIN(Avm2OpNewArray)
		VM_END()

		VM_BEGIN(Avm2OpNewCatch)
		VM_END()

		VM_BEGIN(Avm2OpNewClass)

			uint32_t index = decodeU30(pc);
			const ClassInfo& ci = m_abcFile.classes[index];

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
			operationStack.pop();
		VM_END()

		VM_BEGIN(Avm2OpPopScope)
			scopeStack.pop_back();
		VM_END()

		VM_BEGIN(Avm2OpPushByte)
			int32_t value = *pc++;
			operationStack.push(ActionValue(value));
		VM_END()

		VM_BEGIN(Avm2OpPushDouble)
			uint32_t index = decodeU30(pc);
			operationStack.push(ActionValue(float(m_abcFile.cpool.doubles[index])));
		VM_END()

		VM_BEGIN(Avm2OpPushFalse)
			operationStack.push(ActionValue(false));
		VM_END()

		VM_BEGIN(Avm2OpPushInt)
			uint32_t index = decodeU30(pc);
			operationStack.push(ActionValue(m_abcFile.cpool.s32[index]));
		VM_END()

		VM_BEGIN(Avm2OpPushNamespace)
		VM_END()

		VM_BEGIN(Avm2OpPushNan)
			operationStack.push(ActionValue(0.0f));
		VM_END()

		VM_BEGIN(Avm2OpPushNull)
			operationStack.push(ActionValue());
		VM_END()

		VM_BEGIN(Avm2OpPushScope)
			const ActionValue& v = operationStack.pop();
			scopeStack.push_back(v.getObjectAlways(frame->getContext()));
		VM_END()

		VM_BEGIN(Avm2OpPushShort)
			uint32_t value = decodeU30(pc);
			operationStack.push(ActionValue(int32_t(value)));
		VM_END()

		VM_BEGIN(Avm2OpPushString)
			uint32_t index = decodeU30(pc);
			operationStack.push(ActionValue(m_abcFile.cpool.strings[index]));
		VM_END()

		VM_BEGIN(Avm2OpPushTrue)
			operationStack.push(ActionValue(true));
		VM_END()

		VM_BEGIN(Avm2OpPushUInt)
			uint32_t index = decodeU30(pc);
			operationStack.push(ActionValue(int32_t(m_abcFile.cpool.u32[index])));
		VM_END()

		VM_BEGIN(Avm2OpPushUndefined)
			operationStack.push(ActionValue());
		VM_END()

		VM_BEGIN(Avm2OpPushWith)
		VM_END()

		VM_BEGIN(Avm2OpReturnValue)
			goto __exit_loop;
		VM_END()

		VM_BEGIN(Avm2OpReturnVoid)
			goto __exit_loop;
		VM_END()

		VM_BEGIN(Avm2OpRShift)
		VM_END()

		VM_BEGIN(Avm2OpSetLocal)
			uint32_t index = decodeU30(pc);
			frame->setRegister(index, operationStack.pop());
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_0)
			frame->setRegister(0, operationStack.pop());
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_1)
			frame->setRegister(1, operationStack.pop());
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_2)
			frame->setRegister(2, operationStack.pop());
		VM_END()

		VM_BEGIN(Avm2OpSetLocal_3)
			frame->setRegister(3, operationStack.pop());
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
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			operationStack.push(value1 - value2);
		VM_END()

		VM_BEGIN(Avm2OpSubtract_i)
			ActionValue value2 = operationStack.pop();
			ActionValue value1 = operationStack.pop();
			int32_t result = value1.getInteger() - value2.getInteger();
			operationStack.push(ActionValue(result));
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
__exit_loop:;
}

void ActionVMImage2::serialize(ISerializer& s)
{
}

	}
}
