/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_CgEmitter_H
#define traktor_render_CgEmitter_H

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

#endif	// traktor_render_CgEmitter_H
