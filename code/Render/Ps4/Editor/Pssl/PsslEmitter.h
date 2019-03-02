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
class PsslContext;

/*!
 * \ingroup GNM
 */
class PsslEmitter
{
public:
	PsslEmitter();

	virtual ~PsslEmitter();

	bool emit(PsslContext& c, Node* node);

private:
	std::map< const TypeInfo*, Emitter* > m_emitters;
};

	}
}

