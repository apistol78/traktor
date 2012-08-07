#if defined(_WIN32)
#	include <windows.h>
#	define CC WINAPI
#endif
#include <npapi.h>
#include <npfunctions.h>

#if !defined(CC)
#	define CC
#endif

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* funcs)
{
	if (funcs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if (funcs->size < sizeof(NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	funcs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	//funcs->newp          = NPP_New;
	//funcs->destroy       = NPP_Destroy;
	//funcs->setwindow     = NPP_SetWindow;
	//funcs->newstream     = NPP_NewStream;
	//funcs->destroystream = NPP_DestroyStream;
	//funcs->asfile        = NPP_StreamAsFile;
	//funcs->writeready    = NPP_WriteReady;
	//funcs->write         = NPP_Write;
	//funcs->print         = NPP_Print;
	//funcs->event         = NPP_HandleEvent;
	//funcs->urlnotify     = NPP_URLNotify;
	//funcs->getvalue      = NPP_GetValue;
	//funcs->setvalue      = NPP_SetValue;
	funcs->javaClass     = NULL;

	return NPERR_NO_ERROR;
}

#if defined(_WIN32)
NPError OSCALL NP_Initialize(NPNetscapeFuncs* npnfuncs)
#else
NPError OSCALL NP_Initialize(NPNetscapeFuncs* npnfuncs, NPPluginFuncs* nppfuncs)
#endif
{
#if !defined(_WIN32)
	NPError error = NP_GetEntryPoints(nppfuncs);
	if (error != NPERR_NO_ERROR)
		return error;
#endif

	return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
	return NPERR_NO_ERROR;
}

const char* NP_GetMIMEDescription()
{
	return "";
}
