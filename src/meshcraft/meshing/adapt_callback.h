//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef ADAPT_CALLBACK_H____4114EFCF_10D1_4494_BF86_748E3D448ECC__INCLUDED_
#define ADAPT_CALLBACK_H____4114EFCF_10D1_4494_BF86_748E3D448ECC__INCLUDED_
 
#include "MSCMESHGUID.h"

void license_cb( MSCMESHGUID *client_guid);
void message_cb(char *msg,int severity, int *, double *, void *user_data);
void interrupt_cb(int *interrupt_status, void *user_data);
void progress_cb(int type, int percent_complete,void *user_data);

#endif