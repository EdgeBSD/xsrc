/* $XFree86: xc/programs/Xserver/record/recordmod.c,v 1.5 1999/01/26 05:54:21 dawes Exp $ */

#include "xf86Module.h"

extern Bool noTestExtensions;

static MODULESETUPPROTO(recordSetup);

extern void RecordExtensionInit(INITARGS);

ExtensionModule recordExt = {
    RecordExtensionInit,
    "RECORD",
    &noTestExtensions,
    NULL,
    NULL
};

static XF86ModuleVersionInfo VersRec = {
	"record",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	1, 13, 0,
	ABI_CLASS_EXTENSION,
	ABI_EXTENSION_VERSION,
	MOD_CLASS_EXTENSION,
	{0,0,0,0}
};

XF86ModuleData recordModuleData = { &VersRec, recordSetup, NULL };

static pointer
recordSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    LoadExtension(&recordExt, FALSE);

    /* Need a non-NULL return value to indicate success */
    return (pointer)1;
}

