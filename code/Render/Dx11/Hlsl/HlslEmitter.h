#ifndef traktor_render_HlslEmitter_H
#define traktor_render_HlslEmitter_H

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

#endif	// traktor_render_HlslEmitter_H
