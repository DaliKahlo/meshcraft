//$c1   XRL 07/22/2011 Created
//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//
#include <stdio.h>
#include "adapt_callback.h"
#include "meshkey.h"

void license_cb(MSCMESHGUID *client_guid) 
{
    *client_guid = MSC_TETADAPT_guid;
}

void message_cb(char *msg,int severity, int *data, double *ddata, void *user_data) 
{
	switch (severity) {
	case 3000:
		printf("*** Warning:\t%s\n",msg);
		break;
	case 3001:
		printf("***** Error:\t%s\n",msg);
		break;
	case 3002:
		printf("Fatal Error:\t%s\n",msg);
		break;
	case 3003:
		printf("%s\n",msg);
		break;
	case 3004:
		printf("%s\n",msg);
		break;
	default:
		printf("%s\n",msg);
	}
}


void interrupt_cb(int *interrupt_status,void *user_data)
{
  if(user_data) {
	  bool *you_want_to_interrupt = (bool *) user_data;

	  if(*you_want_to_interrupt)
		*interrupt_status = 1;  /* you want to stop tessadapt */
	  else 
		*interrupt_status = 0;
  }

  return;
}

// process_type:	1		create discrete topology 
//					2		smooth piecewise mesh field
//					3		initial mesh coarsening
//					4		intelligent mesh refinement
//					5		mesh enrichment
void progress_cb(int process_type, int percent_complete,void *user_data)
{
  return;
}
