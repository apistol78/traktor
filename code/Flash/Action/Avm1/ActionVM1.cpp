#include <cstring>
#include <cmath>
#include "Core/Io/BitReader.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionDictionary.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/Avm1/ActionOpcodes.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionVM1.h"
#include "Flash/Action/Avm1/ActionVMImage1.h"
#include "Flash/Action/Avm1/ActionVMTrace1.h"

#define T_TRACE_EXECUTE 0

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ActionVM1", 0, ActionVM1, IActionVM)

ActionVM1::ActionVM1()
{
	m_timer.start();

#if T_TRACE_EXECUTE
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

	return image;
}

void ActionVM1::execute(ActionFrame* frame) const
{
	const ActionVMImage1* image = static_cast< const ActionVMImage1* >(frame->getImage());
	T_ASSERT (image);

	// Prepare image on first execution.
	if (!image->m_prepared)
	{
		const_cast< ActionVMImage1* >(image)->prepare();
		T_ASSERT (image->m_prepared);
	}

	// Setup execution state.
	ExecutionState state;
	state.image = image;
	state.frame = frame;
	state.pc = image->m_byteCode.c_ptr();
	state.npc = state.pc + 1;
	state.data = 0;
	state.length = 0;
	state.timer = &m_timer;

	// Cache frequently used instances.
	state.context = frame->getContext();
	state.self = frame->getSelf();
	state.global = frame->getContext()->getGlobal();
	state.movieClip = frame->getContext()->getMovieClip();
	state.trace = 0;

#if T_TRACE_EXECUTE
	if (m_trace)
	{
		m_trace->beginDispatcher();
		state.trace = &m_trace->getTraceStream();

		const ActionDictionary* dictionary = state.frame->getDictionary();
		if (dictionary)
		{
			*state.trace << L"Dictionary:" << Endl;
			*state.trace << IncreaseIndent;

			const AlignedVector< ActionValue >& table = dictionary->getTable();
			for (uint32_t i = 0; i < table.size(); ++i)
				*state.trace << i << L". " << table[i].getWideString() << Endl;

			*state.trace << DecreaseIndent;
		}
	}
#endif

	const uint8_t* end = state.pc + image->m_byteCode.size();
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
		if (m_trace)
			m_trace->preDispatch(state, info);
#endif
		info.execute(state);
#if T_TRACE_EXECUTE
		if (m_trace)
			m_trace->postDispatch(state, info);
#endif

		// Update program counter.
		state.pc = state.npc;
	}

#if T_TRACE_EXECUTE
	if (m_trace)
		m_trace->endDispatcher();
#endif
}

void ActionVM1::serialize(ISerializer& s)
{
}

	}
}
