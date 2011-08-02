
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "resource.h"
#include "scrnintstr.h"
#include "extnsionst.h"
#include "servermd.h"
#include <X11/Xfuncproto.h>
#include "xvdix.h"
#include <X11/extensions/XvMC.h>
#include <X11/extensions/Xvproto.h>
#include <X11/extensions/XvMCproto.h>
#include "xvmcext.h"
#include "protocol-versions.h"

#ifdef HAS_XVMCSHM
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#endif /* HAS_XVMCSHM */
   


#define DR_CLIENT_DRIVER_NAME_SIZE 48
#define DR_BUSID_SIZE 48

static DevPrivateKeyRec XvMCScreenKeyRec;
#define XvMCScreenKey (&XvMCScreenKeyRec)
static Bool XvMCInUse;

unsigned long XvMCGeneration = 0;

int XvMCReqCode;
int XvMCEventBase;

unsigned long XvMCRTContext;
unsigned long XvMCRTSurface;
unsigned long XvMCRTSubpicture;

typedef struct {
   int num_adaptors;
   XvMCAdaptorPtr adaptors;
   CloseScreenProcPtr	CloseScreen;
   char clientDriverName[DR_CLIENT_DRIVER_NAME_SIZE];
   char busID[DR_BUSID_SIZE];
   int major;
   int minor;
   int patchLevel;
} XvMCScreenRec, *XvMCScreenPtr; 

#define XVMC_GET_PRIVATE(pScreen) \
    (XvMCScreenPtr)(dixLookupPrivate(&(pScreen)->devPrivates, XvMCScreenKey))


static int
XvMCDestroyContextRes(pointer data, XID id)
{
   XvMCContextPtr pContext = (XvMCContextPtr)data;
   
   pContext->refcnt--;

   if(!pContext->refcnt) {
  	 XvMCScreenPtr pScreenPriv = XVMC_GET_PRIVATE(pContext->pScreen);
	(*pScreenPriv->adaptors[pContext->adapt_num].DestroyContext)(pContext);
	free(pContext);
   }	   

   return Success;
}

static int
XvMCDestroySurfaceRes(pointer data, XID id)
{
   XvMCSurfacePtr pSurface = (XvMCSurfacePtr)data;
   XvMCContextPtr pContext = pSurface->context;
   XvMCScreenPtr pScreenPriv = XVMC_GET_PRIVATE(pContext->pScreen);

   (*pScreenPriv->adaptors[pContext->adapt_num].DestroySurface)(pSurface); 
   free(pSurface);

   XvMCDestroyContextRes((pointer)pContext, pContext->context_id);

   return Success;
}


static int
XvMCDestroySubpictureRes(pointer data, XID id)
{
   XvMCSubpicturePtr pSubpict = (XvMCSubpicturePtr)data;
   XvMCContextPtr pContext = pSubpict->context;
   XvMCScreenPtr pScreenPriv = XVMC_GET_PRIVATE(pContext->pScreen);

   (*pScreenPriv->adaptors[pContext->adapt_num].DestroySubpicture)(pSubpict); 
   free(pSubpict);

   XvMCDestroyContextRes((pointer)pContext, pContext->context_id);

   return Success;
}

static int 
ProcXvMCQueryVersion(ClientPtr client)
{
    xvmcQueryVersionReply rep;
    /* REQUEST(xvmcQueryVersionReq); */
    REQUEST_SIZE_MATCH(xvmcQueryVersionReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.major = SERVER_XVMC_MAJOR_VERSION;
    rep.minor = SERVER_XVMC_MINOR_VERSION;
    WriteToClient(client, sizeof(xvmcQueryVersionReply), (char*)&rep);
    return Success;
}


static int 
ProcXvMCListSurfaceTypes(ClientPtr client)
{
    XvPortPtr pPort;
    int i;
    XvMCScreenPtr pScreenPriv;
    xvmcListSurfaceTypesReply rep;
    xvmcSurfaceInfo info;
    XvMCAdaptorPtr adaptor = NULL;
    XvMCSurfaceInfoPtr surface;
    REQUEST(xvmcListSurfaceTypesReq);
    REQUEST_SIZE_MATCH(xvmcListSurfaceTypesReq);

    VALIDATE_XV_PORT(stuff->port, pPort, DixReadAccess);

    if(XvMCInUse) { /* any adaptors at all */
       ScreenPtr pScreen = pPort->pAdaptor->pScreen;
       if((pScreenPriv = XVMC_GET_PRIVATE(pScreen))) {  /* any this screen */
          for(i = 0; i < pScreenPriv->num_adaptors; i++) {
             if(pPort->pAdaptor == pScreenPriv->adaptors[i].xv_adaptor) {
               adaptor = &(pScreenPriv->adaptors[i]);
               break;
             }
          }
       }
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num = (adaptor) ? adaptor->num_surfaces : 0;
    rep.length = bytes_to_int32(rep.num * sizeof(xvmcSurfaceInfo));
 
    WriteToClient(client, sizeof(xvmcListSurfaceTypesReply), (char*)&rep);

    for(i = 0; i < rep.num; i++) {
	surface = adaptor->surfaces[i];
	info.surface_type_id = surface->surface_type_id;
	info.chroma_format = surface->chroma_format;
	info.max_width = surface->max_width;
	info.max_height = surface->max_height;
	info.subpicture_max_width = surface->subpicture_max_width;
	info.subpicture_max_height = surface->subpicture_max_height;
	info.mc_type = surface->mc_type;
	info.flags = surface->flags;
	WriteToClient(client, sizeof(xvmcSurfaceInfo), (char*)&info);
    }

    return Success;
}

static int 
ProcXvMCCreateContext(ClientPtr client)
{
    XvPortPtr pPort;
    CARD32 *data = NULL;
    int dwords = 0; 
    int i, result, adapt_num = -1;
    ScreenPtr pScreen;
    XvMCContextPtr pContext;
    XvMCScreenPtr pScreenPriv;
    XvMCAdaptorPtr adaptor = NULL;
    XvMCSurfaceInfoPtr surface = NULL;
    xvmcCreateContextReply rep;
    REQUEST(xvmcCreateContextReq);
    REQUEST_SIZE_MATCH(xvmcCreateContextReq);

    VALIDATE_XV_PORT(stuff->port, pPort, DixReadAccess);

    pScreen = pPort->pAdaptor->pScreen;

    if(!XvMCInUse) /* no XvMC adaptors */
       return BadMatch;
 
    if(!(pScreenPriv = XVMC_GET_PRIVATE(pScreen))) /* none this screen */
       return BadMatch;

    for(i = 0; i < pScreenPriv->num_adaptors; i++) {
	if(pPort->pAdaptor == pScreenPriv->adaptors[i].xv_adaptor) {
	    adaptor = &(pScreenPriv->adaptors[i]);
	    adapt_num = i; 
	    break;
	}
    }

    if(adapt_num < 0) /* none this port */
	return BadMatch;	

    for(i = 0; i < adaptor->num_surfaces; i++) {
        if(adaptor->surfaces[i]->surface_type_id == stuff->surface_type_id) {
            surface = adaptor->surfaces[i];
            break;
        }
    }

    /* adaptor doesn't support this suface_type_id */
    if(!surface) return BadMatch;


    if((stuff->width > surface->max_width) ||
       (stuff->height > surface->max_height))
        return BadValue;

    if(!(pContext = malloc(sizeof(XvMCContextRec)))) {
	return BadAlloc;
    }


    pContext->pScreen = pScreen;
    pContext->adapt_num = adapt_num;
    pContext->context_id = stuff->context_id;
    pContext->surface_type_id = stuff->surface_type_id;
    pContext->width = stuff->width;
    pContext->height = stuff->height;
    pContext->flags = stuff->flags;
    pContext->refcnt = 1;

    result = (*adaptor->CreateContext)(pPort, pContext, &dwords, &data);

    if(result != Success) {
	free(pContext);
	return result;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.width_actual = pContext->width; 
    rep.height_actual = pContext->height; 
    rep.flags_return = pContext->flags; 
    rep.length = dwords;

    WriteToClient(client, sizeof(xvmcCreateContextReply), (char*)&rep);
    if(dwords)
      WriteToClient(client, dwords << 2, (char*)data); 
    AddResource(pContext->context_id, XvMCRTContext, pContext);

    free(data);

    return Success;
}

static int 
ProcXvMCDestroyContext(ClientPtr client)
{
    pointer val;
    int rc;
    REQUEST(xvmcDestroyContextReq);
    REQUEST_SIZE_MATCH(xvmcDestroyContextReq);

    rc = dixLookupResourceByType(&val, stuff->context_id, XvMCRTContext,
				 client, DixDestroyAccess);
    if (rc != Success)
	return rc;

    FreeResource(stuff->context_id, RT_NONE); 

    return Success;
}

static int 
ProcXvMCCreateSurface(ClientPtr client)
{
    CARD32 *data = NULL;
    int dwords = 0;
    int result;
    XvMCContextPtr pContext;
    XvMCSurfacePtr pSurface;
    XvMCScreenPtr pScreenPriv;
    xvmcCreateSurfaceReply rep;
    REQUEST(xvmcCreateSurfaceReq);
    REQUEST_SIZE_MATCH(xvmcCreateSurfaceReq);

    result = dixLookupResourceByType((pointer *)&pContext, stuff->context_id,
				     XvMCRTContext, client, DixUseAccess);
    if (result != Success)
        return result;

    pScreenPriv = XVMC_GET_PRIVATE(pContext->pScreen);

    if(!(pSurface = malloc(sizeof(XvMCSurfaceRec)))) 
        return BadAlloc;

    pSurface->surface_id = stuff->surface_id;
    pSurface->surface_type_id = pContext->surface_type_id;
    pSurface->context = pContext;

    result = (*pScreenPriv->adaptors[pContext->adapt_num].CreateSurface)(
                pSurface, &dwords, &data);

    if(result != Success) {
        free(pSurface);
        return result;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = dwords;

    WriteToClient(client, sizeof(xvmcCreateSurfaceReply), (char*)&rep);
    if(dwords)
      WriteToClient(client, dwords << 2, (char*)data);
    AddResource(pSurface->surface_id, XvMCRTSurface, pSurface);

    free(data);

    pContext->refcnt++;

    return Success;
}

static int 
ProcXvMCDestroySurface(ClientPtr client)
{
    pointer val;
    int rc;
    REQUEST(xvmcDestroySurfaceReq);
    REQUEST_SIZE_MATCH(xvmcDestroySurfaceReq);

    rc = dixLookupResourceByType(&val, stuff->surface_id, XvMCRTSurface,
				 client, DixDestroyAccess);
    if (rc != Success)
        return rc;

    FreeResource(stuff->surface_id, RT_NONE);

    return Success;
}

static int 
ProcXvMCCreateSubpicture(ClientPtr client)
{
    Bool image_supported = FALSE;
    CARD32 *data = NULL;
    int i, result, dwords = 0;
    XvMCContextPtr pContext;
    XvMCSubpicturePtr pSubpicture;
    XvMCScreenPtr pScreenPriv;
    xvmcCreateSubpictureReply rep;
    XvMCAdaptorPtr adaptor;
    XvMCSurfaceInfoPtr surface = NULL;
    REQUEST(xvmcCreateSubpictureReq);
    REQUEST_SIZE_MATCH(xvmcCreateSubpictureReq);

    result = dixLookupResourceByType((pointer *)&pContext, stuff->context_id,
				     XvMCRTContext, client, DixUseAccess);
    if (result != Success)
        return result;

    pScreenPriv = XVMC_GET_PRIVATE(pContext->pScreen);

    adaptor = &(pScreenPriv->adaptors[pContext->adapt_num]); 

    /* find which surface this context supports */
    for(i = 0; i < adaptor->num_surfaces; i++) {
	if(adaptor->surfaces[i]->surface_type_id == pContext->surface_type_id){
	   surface = adaptor->surfaces[i];
	   break;
	}
    } 

    if(!surface) return BadMatch;

    /* make sure this surface supports that xvimage format */
    if(!surface->compatible_subpictures) return BadMatch;

    for(i = 0; i < surface->compatible_subpictures->num_xvimages; i++) {
      if(surface->compatible_subpictures->xvimage_ids[i] == stuff->xvimage_id) {
	   image_supported = TRUE;
	   break;
      }
    }

    if(!image_supported) return BadMatch;

    /* make sure the size is OK */
    if((stuff->width > surface->subpicture_max_width) ||
       (stuff->height > surface->subpicture_max_height))
	return BadValue;

    if(!(pSubpicture = malloc(sizeof(XvMCSubpictureRec))))
        return BadAlloc;

    pSubpicture->subpicture_id = stuff->subpicture_id;
    pSubpicture->xvimage_id = stuff->xvimage_id;
    pSubpicture->width = stuff->width;
    pSubpicture->height = stuff->height;
    pSubpicture->num_palette_entries = 0; /* overwritten by DDX */
    pSubpicture->entry_bytes = 0;         /* overwritten by DDX */
    pSubpicture->component_order[0] = 0;  /* overwritten by DDX */
    pSubpicture->component_order[1] = 0;
    pSubpicture->component_order[2] = 0;
    pSubpicture->component_order[3] = 0;
    pSubpicture->context = pContext;
   
    result = (*pScreenPriv->adaptors[pContext->adapt_num].CreateSubpicture)(
                pSubpicture, &dwords, &data);

    if(result != Success) {
        free(pSubpicture);
        return result;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.width_actual = pSubpicture->width;
    rep.height_actual = pSubpicture->height;
    rep.num_palette_entries = pSubpicture->num_palette_entries;
    rep.entry_bytes = pSubpicture->entry_bytes;
    rep.component_order[0] = pSubpicture->component_order[0];
    rep.component_order[1] = pSubpicture->component_order[1];
    rep.component_order[2] = pSubpicture->component_order[2];
    rep.component_order[3] = pSubpicture->component_order[3];
    rep.length = dwords;

    WriteToClient(client, sizeof(xvmcCreateSubpictureReply), (char*)&rep);
    if(dwords)
      WriteToClient(client, dwords << 2, (char*)data);
    AddResource(pSubpicture->subpicture_id, XvMCRTSubpicture, pSubpicture);

    free(data);

    pContext->refcnt++;

    return Success;
}

static int 
ProcXvMCDestroySubpicture(ClientPtr client)
{
    pointer val;
    int rc;
    REQUEST(xvmcDestroySubpictureReq);
    REQUEST_SIZE_MATCH(xvmcDestroySubpictureReq);

    rc = dixLookupResourceByType(&val, stuff->subpicture_id, XvMCRTSubpicture,
				 client, DixDestroyAccess);
    if (rc != Success)
        return rc;

    FreeResource(stuff->subpicture_id, RT_NONE);

    return Success;
}


static int
ProcXvMCListSubpictureTypes(ClientPtr client)
{
    XvPortPtr pPort;
    xvmcListSubpictureTypesReply rep;
    XvMCScreenPtr pScreenPriv;
    ScreenPtr pScreen;
    XvMCAdaptorPtr adaptor = NULL;
    XvMCSurfaceInfoPtr surface = NULL;
    xvImageFormatInfo info;
    XvImagePtr pImage;
    int i, j;
    REQUEST(xvmcListSubpictureTypesReq);
    REQUEST_SIZE_MATCH(xvmcListSubpictureTypesReq);

    VALIDATE_XV_PORT(stuff->port, pPort, DixReadAccess);

    pScreen = pPort->pAdaptor->pScreen;

    if (!dixPrivateKeyRegistered(XvMCScreenKey))
        return BadMatch;   /* No XvMC adaptors */

    if(!(pScreenPriv = XVMC_GET_PRIVATE(pScreen)))
        return BadMatch;   /* None this screen */

    for(i = 0; i < pScreenPriv->num_adaptors; i++) {
        if(pPort->pAdaptor == pScreenPriv->adaptors[i].xv_adaptor) {
            adaptor = &(pScreenPriv->adaptors[i]);
            break;
        }
    }

    if(!adaptor) return BadMatch;

    for(i = 0; i < adaptor->num_surfaces; i++) {
        if(adaptor->surfaces[i]->surface_type_id == stuff->surface_type_id) {
            surface = adaptor->surfaces[i];
            break;
        }
    }

    if(!surface) return BadMatch;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num = 0;
    if(surface->compatible_subpictures) 
	rep.num = surface->compatible_subpictures->num_xvimages;

    rep.length = bytes_to_int32(rep.num * sizeof(xvImageFormatInfo));

    WriteToClient(client, sizeof(xvmcListSubpictureTypesReply), (char*)&rep);

    for(i = 0; i < rep.num; i++) {
	pImage = NULL;
	for(j = 0; j < adaptor->num_subpictures; j++) {
	    if(surface->compatible_subpictures->xvimage_ids[i] ==
	       adaptor->subpictures[j]->id)
	    {
		pImage = adaptor->subpictures[j];
	        break;
	    }
	}
	if(!pImage) return BadImplementation;

        info.id = pImage->id;      
	info.type = pImage->type;  
        info.byte_order = pImage->byte_order; 
        memcpy(&info.guid, pImage->guid, 16);      
        info.bpp = pImage->bits_per_pixel;         
        info.num_planes = pImage->num_planes;      
        info.depth = pImage->depth;        
        info.red_mask = pImage->red_mask;  
        info.green_mask = pImage->green_mask;      
        info.blue_mask = pImage->blue_mask;        
        info.format = pImage->format;      
        info.y_sample_bits = pImage->y_sample_bits;        
        info.u_sample_bits = pImage->u_sample_bits;        
        info.v_sample_bits = pImage->v_sample_bits;        
        info.horz_y_period = pImage->horz_y_period;        
        info.horz_u_period = pImage->horz_u_period;        
        info.horz_v_period = pImage->horz_v_period;        
        info.vert_y_period = pImage->vert_y_period;        
        info.vert_u_period = pImage->vert_u_period;        
        info.vert_v_period = pImage->vert_v_period;        
        memcpy(&info.comp_order, pImage->component_order, 32);     
        info.scanline_order = pImage->scanline_order;
	WriteToClient(client, sizeof(xvImageFormatInfo), (char*)&info);
    }

    return Success;
}

static int
ProcXvMCGetDRInfo(ClientPtr client)
{
    xvmcGetDRInfoReply rep;
    XvPortPtr pPort;
    ScreenPtr pScreen;
    XvMCScreenPtr pScreenPriv;

#ifdef HAS_XVMCSHM
    volatile CARD32 *patternP;
#endif

    REQUEST(xvmcGetDRInfoReq);
    REQUEST_SIZE_MATCH(xvmcGetDRInfoReq);

    VALIDATE_XV_PORT(stuff->port, pPort, DixReadAccess);

    pScreen = pPort->pAdaptor->pScreen;
    pScreenPriv = XVMC_GET_PRIVATE(pScreen);
    
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.major = pScreenPriv->major;
    rep.minor = pScreenPriv->minor;
    rep.patchLevel = pScreenPriv->patchLevel;
    rep.nameLen = bytes_to_int32(strlen(pScreenPriv->clientDriverName) + 1);
    rep.busIDLen = bytes_to_int32(strlen(pScreenPriv->busID) + 1);

    rep.length = rep.nameLen + rep.busIDLen;
    rep.nameLen <<=2;
    rep.busIDLen <<=2;

    /*
     * Read back to the client what she has put in the shared memory
     * segment she prepared for us.
     */

    rep.isLocal = 1;
#ifdef HAS_XVMCSHM
    patternP = (CARD32 *)shmat( stuff->shmKey, NULL, SHM_RDONLY );
    if ( -1 != (long) patternP) {
        volatile CARD32 *patternC = patternP;
	int i;
	CARD32 magic = stuff->magic;
	
	rep.isLocal = 1;
	i = 1024 / sizeof(CARD32);
	
	while ( i-- ) {
	    if (*patternC++ != magic) {
		rep.isLocal = 0;
		break;
	    }
	    magic = ~magic;
	}
	shmdt( (char *)patternP ); 
    }
#endif /* HAS_XVMCSHM */
    
    WriteToClient(client, sizeof(xvmcGetDRInfoReply), 
		  (char*)&rep);
    if (rep.length) {      
	WriteToClient(client, rep.nameLen, 
		      pScreenPriv->clientDriverName);
	WriteToClient(client, rep.busIDLen, 
		      pScreenPriv->busID);
    }	
    return Success;
}


int (*ProcXvMCVector[xvmcNumRequest])(ClientPtr) = {
  ProcXvMCQueryVersion,
  ProcXvMCListSurfaceTypes,
  ProcXvMCCreateContext,
  ProcXvMCDestroyContext,
  ProcXvMCCreateSurface,
  ProcXvMCDestroySurface,
  ProcXvMCCreateSubpicture,
  ProcXvMCDestroySubpicture,
  ProcXvMCListSubpictureTypes,
  ProcXvMCGetDRInfo
};

static int
ProcXvMCDispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    if(stuff->data < xvmcNumRequest)
	return (*ProcXvMCVector[stuff->data])(client);
    else
	return BadRequest;
}

static int
SProcXvMCDispatch (ClientPtr client)
{
    /* We only support local */
    return BadImplementation;
}

void
XvMCExtensionInit(void)
{
   ExtensionEntry *extEntry;

   if (!dixPrivateKeyRegistered(XvMCScreenKey))
	return;

   if(!(XvMCRTContext = CreateNewResourceType(XvMCDestroyContextRes,
					      "XvMCRTContext")))
	return;

   if(!(XvMCRTSurface = CreateNewResourceType(XvMCDestroySurfaceRes,
					      "XvMCRTSurface")))
	return;

   if(!(XvMCRTSubpicture = CreateNewResourceType(XvMCDestroySubpictureRes,
						 "XvMCRTSubpicture")))
	return;

   extEntry = AddExtension(XvMCName, XvMCNumEvents, XvMCNumErrors, 
                              ProcXvMCDispatch, SProcXvMCDispatch,
                              NULL, StandardMinorOpcode);

   if(!extEntry) return;
  
   XvMCReqCode = extEntry->base;
   XvMCEventBase = extEntry->eventBase;
   SetResourceTypeErrorValue(XvMCRTContext, extEntry->errorBase + XvMCBadContext);
   SetResourceTypeErrorValue(XvMCRTSurface, extEntry->errorBase + XvMCBadSurface);
   SetResourceTypeErrorValue(XvMCRTSubpicture, extEntry->errorBase + XvMCBadSubpicture);
}

static Bool
XvMCCloseScreen (int i, ScreenPtr pScreen)
{
    XvMCScreenPtr pScreenPriv = XVMC_GET_PRIVATE(pScreen);

    pScreen->CloseScreen = pScreenPriv->CloseScreen;

    free(pScreenPriv);

    return (*pScreen->CloseScreen)(i, pScreen);
}


int
XvMCScreenInit(ScreenPtr pScreen, int num, XvMCAdaptorPtr pAdapt)
{
   XvMCScreenPtr pScreenPriv;

   if (!dixRegisterPrivateKey(&XvMCScreenKeyRec, PRIVATE_SCREEN, 0))
       return BadAlloc;

   if(!(pScreenPriv = malloc(sizeof(XvMCScreenRec))))
	return BadAlloc;

   dixSetPrivate(&pScreen->devPrivates, XvMCScreenKey, pScreenPriv);

   pScreenPriv->CloseScreen = pScreen->CloseScreen;
   pScreen->CloseScreen = XvMCCloseScreen;

   pScreenPriv->num_adaptors = num;
   pScreenPriv->adaptors = pAdapt;
   pScreenPriv->clientDriverName[0] = 0;
   pScreenPriv->busID[0] = 0;
   pScreenPriv->major = 0;
   pScreenPriv->minor = 0;
   pScreenPriv->patchLevel = 0;

   XvMCInUse = TRUE;

   return Success;
}

XvImagePtr XvMCFindXvImage(XvPortPtr pPort, CARD32 id)
{
    XvImagePtr pImage = NULL;
    ScreenPtr pScreen = pPort->pAdaptor->pScreen;
    XvMCScreenPtr pScreenPriv;
    XvMCAdaptorPtr adaptor = NULL;
    int i;

    if (!dixPrivateKeyRegistered(XvMCScreenKey))
        return NULL;

    if(!(pScreenPriv = XVMC_GET_PRIVATE(pScreen))) 
        return NULL;

    for(i = 0; i < pScreenPriv->num_adaptors; i++) {
       if(pPort->pAdaptor == pScreenPriv->adaptors[i].xv_adaptor) {
          adaptor = &(pScreenPriv->adaptors[i]);
          break;
       }
    }

    if(!adaptor) return NULL;

    for(i = 0; i < adaptor->num_subpictures; i++) {
        if(adaptor->subpictures[i]->id == id) {
            pImage = adaptor->subpictures[i];
            break;
        }
    }

    return pImage;
}

int
xf86XvMCRegisterDRInfo(ScreenPtr pScreen, char *name,
		       char *busID, int major, int minor,
		       int patchLevel)
{
    XvMCScreenPtr pScreenPriv = XVMC_GET_PRIVATE(pScreen);
    strncpy(pScreenPriv->clientDriverName, name,
	    DR_CLIENT_DRIVER_NAME_SIZE);
    strncpy(pScreenPriv->busID, busID, DR_BUSID_SIZE);
    pScreenPriv->major = major;
    pScreenPriv->minor = minor;
    pScreenPriv->patchLevel = patchLevel;
    pScreenPriv->clientDriverName[DR_CLIENT_DRIVER_NAME_SIZE-1] = 0;
    pScreenPriv->busID[DR_BUSID_SIZE-1] = 0;
    return Success;
}

