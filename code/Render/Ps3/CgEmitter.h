#ifndef traktor_render_CgEmitter_H
#define traktor_render_CgEmitter_H

#include <map>

namespace traktor
{

class Type;

	namespace render
	{
		namespace
		{

struct Emitter;

		}

class Node;
class CgContext;

class CgEmitter
{
public:
	CgEmitter();

	virtual ~CgEmitter();

	void emit(CgContext& c, Node* node);

private:
	std::map< const Type*, Emitter* > m_emitters;
};

	}
}

#endif	// traktor_render_CgEmitter_H
