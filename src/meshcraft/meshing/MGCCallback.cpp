//$c1   XRL 07/27/2012 linked mgc_interrupt_cb with mesh progress dialog.
//$c1   XRL 07/22/2011 Created
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include <stdio.h>
#include "MGCCallback.h"
#include "mgckey.h"
#include "mgc_api.h"
#include "mgc_const.h"
#include "ErrorData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void mgc_license(MSCMESHGUID *client_guid) 
{
    *client_guid = MTB_MSC_DisteneMesh_guid;
}

bool mgc_message_cb(pMessage_t msg, void *user_data)
{
  MGCMessageType msgtype = MGC_message_get_type(msg);
  MGCMessageId   msgid   = MGC_message_get_id(msg);
  char * msgstr = MGC_message_get_description(msg);

  if( msgid == MGC_MSG_STRING ) {
	  switch( msgtype ) {
		  case MGC_MSG_TYPE_INFOR:  
			  { printf(msgstr); break; }
		  case MGC_MSG_TYPE_WARN:
			  { printf("Warning: %s",msgstr); break; }
	  }
  }

  if(user_data) {
	  CErrData * pErrData = (CErrData *)user_data;
	  if(pErrData && msgtype == MGC_MSG_TYPE_FATAL) {
		  int n, data[8];
		  pErrData->addCode((int)msgid);
		  MGC_message_get_integer_data(msg, &n, data);
		  switch(msgid) {
		  case MGC_MSG_COINCIDENT_NODES:
			  pErrData->addEdge(data[0],data[1]);
			  break;
		  case MGC_MSG_FACE_INTERSECT:
			  pErrData->addFace(data[0],data[1],data[2]);
			  pErrData->addFace(data[3],data[4],data[5]);
			  break;
		  case MGC_MSG_EDGE_INTERSECT:
			  pErrData->addEdge(data[0],data[1]);
			  pErrData->addEdge(data[2],data[3]);
			  break;
		  case MGC_MSG_EDGE_FACE_INTERSECT:
			  pErrData->addEdge(data[0],data[1]);
			  pErrData->addFace(data[2],data[3],data[4]);
			  break;
		  case MGC_MSG_POINT_ON_FACE:
			  pErrData->addFace(data[1],data[2],data[3]);
			  break;
		  case MGC_MSG_EDGE_MISSING:
			   pErrData->addEdge(data[0],data[1]);
			   break;
		  }
	  }
  }

  return true;
}

int mgc_interrupt_cb(int *interrupt_status, void *user_data)
{
  bool *you_want_to_interrupt = (bool *) user_data;

  if(*you_want_to_interrupt)
	*interrupt_status = MGC_INTERRUPT_STOP;  /* you want to stop BLSurf */
  else
    *interrupt_status = MGC_INTERRUPT_CONTINUE;    
  
  return MGC_STATUS_OK;
}
