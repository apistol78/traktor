#pragma once

#include <map>

namespace traktor
{

class TypeInfo;

	namespace render
	{
		namespace
		{

struct Emitter;

		}

class Node;
class HlslContext;

/*!
 * \ingroup DX11
 */
class HlslEmitter
{
public:
	HlslEmitter();

	virtual ~HlslEmitter();

	bool emit(HlslContext& c, Node* node);

private:
	std::map< const TypeInfo*, Emitter* > m_emitters;
};

	}
}

