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
class CgContext;

/*!
 * \ingroup PS3
 */
class CgEmitter
{
public:
	CgEmitter();

	virtual ~CgEmitter();

	bool emit(CgContext& c, Node* node);

private:
	std::map< const TypeInfo*, Emitter* > m_emitters;
};

	}
}

