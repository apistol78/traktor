/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_GlslEmitter_H
#define traktor_render_GlslEmitter_H

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

#endif	// traktor_render_GlslEmitter_H
