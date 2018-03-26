//========================================================================//
//              Copyright 2011 (Unpublished Material)                     //
//										                                  //
//========================================================================//

#ifndef DISTENE_PARA_INTERFACE_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define DISTENE_PARA_INTERFACE_H__CD27375B_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include "mgc_typedef.h"

void register_mgc_parasolid_interface();
void delete_mgc_parasolid_interface();

int mgc_edge_ask_faces(pGEdge_t, int *,	pGFace_t **);
int mgc_body_ask_faces(pGBody_t, int *, pGFace_t **);
void mgc_memory_free(pGEntity_t *);
pGEntity_t gid2pGEntity(MGCGeomType,pGEntity_t, int);

#endif