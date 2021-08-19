#pragma once

#include "Core/Ref.h"
#include "Render/Buffer.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class BufferDx11 : public Buffer
{
	T_RTTI_CLASS;

protected:
	Ref< ContextDx11 > m_context;

	explicit BufferDx11(ContextDx11* context, uint32_t bufferSize);
};

	}
}
