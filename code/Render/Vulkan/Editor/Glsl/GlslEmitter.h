#pragma once

#include <map>

namespace traktor
{

class TypeInfo;

	namespace render
	{

class Node;
class GlslContext;

struct Emitter;

/*!
 * \ingroup Vulkan
 */
class GlslEmitter
{
public:
	GlslEmitter();

	virtual ~GlslEmitter();

	bool emit(GlslContext& c, Node* node);

private:
	std::map< const TypeInfo*, Emitter* > m_emitters;
};

	}
}

