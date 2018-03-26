//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef MGC_CALLBACK_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MGC_CALLBACK_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
 
#include "mgc_typedef.h"
#include "MSCMESHGUID.h"

void mgc_license( MSCMESHGUID *client_guid);
bool mgc_message_cb(pMessage_t msg, void *user_data);
int mgc_interrupt_cb(int *interrupt_status, void *user_data);
bool mgc_progress_cb(MGCMeshProgressType type, int percent_complete,void *user_data);

#endif