#include "stdafx.h"
#include "mmerrors.h"

void reportmmresult(const char *str, MMRESULT res)
{
	switch (res)
	{
	case MMSYSERR_NOERROR:
	    printf("%s returns NOERROR.\n", str);
		break;
	case MMSYSERR_ALLOCATED:
	    printf("%s returns ALLOCATED.\n", str);
		break;
	case MMSYSERR_BADDEVICEID:
	    printf("%s returns BADDEVICEID.\n", str);
		break;
	case MMSYSERR_NODRIVER:
	    printf("%s returns NODRIVER.\n", str);
		break;
	case MMSYSERR_NOMEM:
	    printf("%s returns NOMEM.\n", str);
		break;
	case WAVERR_BADFORMAT:
	    printf("%s returns BADFORMAT.\n", str);
		break;
    case MMSYSERR_ERROR:
	    printf("%s returns ERROR.\n", str);
		break;
    case MMSYSERR_NOTENABLED:
	    printf("%s returns NOTENABLED.\n", str);
		break;
    case MMSYSERR_INVALHANDLE:
	    printf("%s returns BADFORMAT.\n", str);
		break;
    case MMSYSERR_NOTSUPPORTED:
	    printf("%s returns NOTSUPPORTED.\n", str);
		break;
    case MMSYSERR_BADERRNUM:
	    printf("%s returns BADERRNUM.\n", str);
		break;
    case MMSYSERR_INVALFLAG:
	    printf("%s returns INVALFLAG.\n", str);
		break;
    case MMSYSERR_INVALPARAM:
	    printf("%s returns INVALPARAM.\n", str);
		break;
    case MMSYSERR_HANDLEBUSY:
	    printf("%s returns HANDLEBUSY.\n", str);
		break;
    case MMSYSERR_INVALIDALIAS:
	    printf("%s returns INVALIDALIAS.\n", str);
		break;
    case MMSYSERR_BADDB:
	    printf("%s returns BADDB.\n", str);
		break;
    case MMSYSERR_KEYNOTFOUND:
	    printf("%s returns KEYNOTFOUND.\n", str);
		break;
    case MMSYSERR_READERROR:
	    printf("%s returns READERROR.\n", str);
		break;
    case MMSYSERR_WRITEERROR:
	    printf("%s returns WRITEERROR.\n", str);
		break;
    case MMSYSERR_DELETEERROR:
	    printf("%s returns DELETEERROR.\n", str);
		break;
    case MMSYSERR_VALNOTFOUND:
	    printf("%s returns VALNOTFOUND.\n", str);
		break;
    case MMSYSERR_NODRIVERCB:
	    printf("%s returns NODRIVERCB.\n", str);
		break;
    case MMSYSERR_MOREDATA:
	    printf("%s returns MOREDATA.\n", str);
		break;
	default:
	    printf("%s returns %ld.\n", str, res);
		break;
	}
}

