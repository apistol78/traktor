#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionObjectRelay.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionSuper.h"
#include "Flash/Action/Avm1/ActionVMTrace1.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

std::wstring describeValue(const ActionValue& v)
{
	StringOutputStream ss;

	if (v.isUndefined())
		ss << L"undefined (void)";
	else if (v.isBoolean())
		ss << v.getWideString() << L" (boolean)";
	else if (v.isNumeric())
		ss <<v.getWideString() << L" (numeric)";
	else if (v.isString())
		ss << L"\"" << v.getWideString() << L"\" (string)";
	else if (v.isObject())
	{
		ActionObject* object = v.getObject();
		if (object)
		{
			ActionFunction* fn = dynamic_type_cast< ActionFunction* >(object);
			if (fn)
			{
				if (is_a< ActionSuper >(fn))
					ss << L"[type Super] (function @" << uint32_t(fn) << L" \"" << mbstows(fn->getName()) << L"\"";
				else
					ss << L"[type Function] (function @" << uint32_t(fn) << L" \"" << mbstows(fn->getName()) << L"\"";
			}
			else
				ss << L"[object Object] (object @" << uint32_t(object);

			IActionObjectRelay* relay = object->getRelay();
			if (relay)
				ss << L", relay @" << uint32_t(relay) << L" " << type_name(relay);
			
			ss << L")";
		}
		else
			ss << L"null (object)";
	}
	else
		ss << L"garbage";

	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVMTrace1", ActionVMTrace1, Object)

ActionVMTrace1::ActionVMTrace1()
{
	Ref< IStream > traceFile = FileSystem::getInstance().open(L"ActionVM1.trace", File::FmWrite);
	m_stream = new FileOutputStream(traceFile, new Utf8Encoding());

	m_traceStream = new StringOutputStream();
}

void ActionVMTrace1::beginDispatcher()
{
	(*m_stream) << L"Begin dispatcher" << Endl;
}

void ActionVMTrace1::endDispatcher()
{
	(*m_stream) << L"End dispatcher" << Endl;
}

void ActionVMTrace1::preDispatch(const ExecutionState& state, const OperationInfo& info)
{
	const ActionValueStack& stack = state.frame->getStack();
	const ActionValueArray& registers = state.frame->getRegisters();

	m_preStack.resize(stack.depth());
	for (int32_t i = 0; i < stack.depth(); ++i)
		m_preStack[i] = stack.top(-i);

	m_preRegisters.resize(registers.size());
	for (uint32_t i = 0; i < registers.size(); ++i)
		m_preRegisters[i] = registers[i];

	m_traceStream->reset();

	(*m_stream) << IncreaseIndent;
}

void ActionVMTrace1::postDispatch(const ExecutionState& state, const OperationInfo& info)
{
	(*m_stream) << DecreaseIndent;

	(*m_stream) << mbstows(info.name) << Endl;
	(*m_stream) << Endl;

	(*m_stream) << L"Stack (pre):" << Endl;
	(*m_stream) << IncreaseIndent;

	for (size_t i = 0; i < m_preStack.size(); ++i)
		(*m_stream) << uint32_t(i) << L". " << describeValue(m_preStack[i]) << Endl;

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"Registers (pre):" << Endl;
	(*m_stream) << IncreaseIndent;

	for (size_t i = 0; i < m_preRegisters.size(); ++i)
	{
		if (!m_preRegisters[i].isUndefined())
			(*m_stream) << uint32_t(i) << L". " << describeValue(m_preRegisters[i]) << Endl;
	}

	(*m_stream) << DecreaseIndent;

	(*m_stream) << Endl;

	(*m_stream) << L"Log:" << Endl;
	(*m_stream) << IncreaseIndent;

	(*m_stream) << m_traceStream->str();

	(*m_stream) << DecreaseIndent;

	(*m_stream) << Endl;

	(*m_stream) << L"Stack (post):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueStack& stack = state.frame->getStack();
	for (int32_t i = 0; i < stack.depth(); ++i)
		(*m_stream) << i << L". " << describeValue(stack.top(-i)) << Endl;

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"Registers (post):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueArray& registers = state.frame->getRegisters();
	for (uint32_t i = 0; i < registers.size(); ++i)
	{
		if (!registers[i].isUndefined())
			(*m_stream) << uint32_t(i) << L". " << describeValue(registers[i]) << Endl;
	}

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"===========================================================" << Endl;
}

OutputStream& ActionVMTrace1::getTraceStream()
{
	return *m_traceStream;
}

	}
}
