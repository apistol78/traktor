#ifndef traktor_flash_AsMovieClipLoader_H
#define traktor_flash_AsMovieClipLoader_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief MovieClipLoader class.
 * \ingroup Flash
 */
class AsMovieClipLoader : public ActionClass
{
	T_RTTI_CLASS(AsMovieClipLoader)

public:
	static Ref< AsMovieClipLoader > getInstance();

private:
	AsMovieClipLoader();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void MovieClipLoader_addListener(CallArgs& ca);

	void MovieClipLoader_getProgress(CallArgs& ca);

	void MovieClipLoader_loadClip(CallArgs& ca);

	void MovieClipLoader_removeListener(CallArgs& ca);

	void MovieClipLoader_unloadClip(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsMovieClipLoader_H
