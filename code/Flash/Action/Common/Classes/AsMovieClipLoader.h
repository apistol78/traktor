/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsMovieClipLoader_H
#define traktor_flash_AsMovieClipLoader_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class SpriteInstance;

/*! \brief MovieClipLoader class.
 * \ingroup Flash
 */
class AsMovieClipLoader : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsMovieClipLoader(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void MovieClipLoader_addListener(CallArgs& ca);

	void MovieClipLoader_getProgress(CallArgs& ca);

	bool MovieClipLoader_loadClip(ActionObject* self, const std::wstring& url, SpriteInstance* target) const;

	void MovieClipLoader_removeListener(CallArgs& ca);

	void MovieClipLoader_unloadClip(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMovieClipLoader_H
