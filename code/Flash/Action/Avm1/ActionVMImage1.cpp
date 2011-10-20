#include "Core/Log/Log.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionOpcodes.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionVMImage1.h"

#if defined(_DEBUG)
#	define T_TRACE_EXECUTE 0
#	define T_TRACE_PREPARE 0
#else
#	define T_TRACE_EXECUTE 0
#	define T_TRACE_PREPARE 0
#endif

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionVMImage1", ActionVMImage1, IActionVMImage)

ActionVMImage1::ActionVMImage1()
{
}

ActionVMImage1::ActionVMImage1(const uint8_t* byteCode, uint32_t length)
{
	if (length > 0)
	{
		m_byteCode.resize(length);
		std::memcpy(m_byteCode.ptr(), byteCode, length);
	}
}

void ActionVMImage1::execute(ActionFrame* frame) const
{
	ExecutionState state;
	state.image = this;
	state.frame = frame;
	state.pc = m_byteCode.c_ptr();
	state.npc = state.pc + 1;
	state.data = 0;
	state.length = 0;

	// Cache frequently used instances.
	state.context = frame->getContext();
	state.self = frame->getSelf();
	state.global = frame->getContext()->getGlobal();
	state.movieClip = frame->getContext()->getMovieClip();

#if T_TRACE_EXECUTE
	log::debug << IncreaseIndent;
#endif

	const uint8_t* end = state.pc + m_byteCode.size();
	while (state.pc < end)
	{
		const uint8_t op = *state.pc;

		if (op == AopEnd || op == AopReturn)
			break;

		state.npc = state.pc + 1;
		state.data = 0;

		// Decode instruction data.
		state.length = 1;
		if (op & 0x80)
		{
			state.length = *reinterpret_cast< const uint16_t* >(state.pc + 1);
			state.data = state.pc + 3;
			state.npc = state.data + state.length;
		}

		// Get instruction handler and dispatch.
		const OperationInfo& info = c_operationInfos[op];
		T_ASSERT (info.op == op);
		T_ASSERT (info.execute != 0);

#if T_TRACE_EXECUTE
		log::debug << uint32_t(state.pc - m_byteCode.c_ptr()) << L": " << mbstows(info.name) << Endl;
#endif
		info.execute(state);

		// Update program counter.
		state.pc = state.npc;
	}

#if T_TRACE_EXECUTE
	log::debug << DecreaseIndent;
#endif
}

void ActionVMImage1::prepare()
{
	PreparationState state;
	state.image = this;
	state.pc = m_byteCode.ptr();
	state.npc = state.pc + 1;
	state.data = 0;
	state.length = 0;

	const uint8_t* end = state.pc + m_byteCode.size();
	while (state.pc < end)
	{
		const uint8_t op = *state.pc;

		if (op == AopEnd || op == AopReturn)
			break;

		state.npc = state.pc + 1;
		state.data = 0;

		// Decode instruction data.
		state.length = 1;
		if (op & 0x80)
		{
			state.length = *reinterpret_cast< const uint16_t* >(state.pc + 1);
			state.data = state.pc + 3;
			state.npc = state.data + state.length;
		}

		// Get instruction preparation handler and dispatch.
		const OperationInfo& info = c_operationInfos[op];
		T_ASSERT (info.op == op);

#if T_TRACE_PREPARE
		log::debug << uint32_t(state.pc - m_byteCode.ptr()) << L": " << mbstows(info.name) << Endl;
#endif

		if (info.prepare)
			info.prepare(state);

		// Update program counter.
		state.pc = state.npc;
	}
}

uint16_t ActionVMImage1::addConstData(const ActionValue& cd)
{
	m_constData.push_back(cd);
	return uint16_t(m_constData.size() - 1);
}

	}
}
