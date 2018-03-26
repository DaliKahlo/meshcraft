//$c22 XRL 05/28/15 add LoadIBL()
//$c21 XRL 02/10/13 read CTRIA6 and CQUAF8 as linear elements without deleting middle nodes
//$c20 XRL 10/09/13 skip empty space in stringToVal()
//$c19 XRL 10/03/13 read and show point clouds
//$c18 XRL 09/30/13 Support free style format of "GRID," and "CTRIA3,"
//$c17 XRL 04/26/13 use stringToVal() in reading GRID* card
//$c16 XRL 04/22/13 support CBAR2* and CTRIA3* 
//$c15 XRL 04/12/13 read CBAR2 into mesh database 
//$c14 XRL 04/01/13 add addPSModel(), and support toBDF of multi-body model
//$c13 XRL 02/02/13 add stringToVal() to correct read BDF from HyperMesh
//$c12 XRL 01/09/13 filter non soild/shell when loading parasolid parts
//$c11 XRL 11/08/12 Used EN_setID to set element id correctly in LoadBDF
//$c10 XRL 10/22/12 New way to read CTRIA3 and GRID card, support empty lines, and time LoadBDF
//$c9  XRL 10/19/12 Save tri/quad in BDF format from surface meshing
//$c8  XRL 10/08/12 modified LoadBDF for tri/quad from SimX and ANSA.
//$c7  XRL 07/14/12 implemented LoadBDF.
//$c6  XRL 07/03/12 Start to Manage MeshControl here.
//$c5  XRL 12/23/11 Support the display and pick of assemblies.
//$c4  XRL 11/23/11 integrate mesh database "PSMesh" to save the generated mesh.
//$c3  XRL 03/22/11 updated to parasolid v23 and integrated BLpara.
//$c2  XRL 02/03/11 added topol/body/part picking and show its tag in GUI.
//$c1  XRL 02/01/11 created.
//=========================================================================
//
// MeshWorksDoc.cpp : interface of the CMeshWorkView class
//
//=========================================================================

#include "stdafx.h"
#include "MeshWorks.h"
#include "MeshWorkDoc.h"
#include "MeshWorkView.h"
#include "MeshWorkOsgView.h"
#include "Meshing/MeshGen.h"
#include "meshing/AssemblyMesh.h"
#include "osg/scenegraphfactory.h"
#include "adaptapi_internal.h"
#include <osgDB/WriteFile>
#include <math.h>

// Parasolid includes
#include "frustrum_tokens.h"
#include "kernel_interface.h"
#include "frustrum_ifails.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

///====== For  nastran bdf reader ===============///
#define        MAX_LINE        128
#define        MAX_SHELL       100
#define		   MAX_ID_SHORT	   99999999
enum BDF_FORMAT { short_format, long_format };
///===============================================///


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkDoc

IMPLEMENT_DYNCREATE(CMeshWorkDoc, CDocument)

BEGIN_MESSAGE_MAP(CMeshWorkDoc, CDocument)
	//{{AFX_MSG_MAP(CMeshWorkDoc)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_SAVEAS, &CMeshWorkDoc::OnFileSaveas)
	ON_COMMAND(ID_FILE_INSERT, &CMeshWorkDoc::OnFileInsert)
END_MESSAGE_MAP()

// Static variables
CMeshWorkView* CMeshWorkDoc::m_view;
PK_BODY_type_t CMeshWorkDoc::m_currentBodyType = 0;
int CMeshWorkDoc::m_psModelIndex = 0;

//int mesh(int n_parts, int *parts, PSMesh **m);
char* CStringToChar(char* lpszStr, CString& strText);

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkDoc construction/destruction

CMeshWorkDoc::CMeshWorkDoc()
{
	// Initialise view data
	m_viewCentre.coord[ 0 ] = 0.0;
	m_viewCentre.coord[ 1 ] = 0.0;
	m_viewCentre.coord[ 2 ] = 0.0;

	m_viewStyle   = Shaded;
	m_scaleFactor = 1.0;
	m_bSphereRad = 1.0;

	// selection manager
	m_picked.clear();
	m_pSelectionMgr = new CSelectionMgr();

	// save current working dir
	DWORD dwRet = GetCurrentDirectory(MAX_PATH, curworkingdir);

	// initialize geometry, control and mesh data
	m_ntopol = 0;
	m_ngeoms = 0;
	m_topols = NULL;
	m_transfs = NULL;
	m_geoms = NULL;
	m_pAsmMeshMgr = NULL;
	m_pMeshCtrl = NULL;
}

CMeshWorkDoc::~CMeshWorkDoc()
{
	delete m_pSelectionMgr;

	clear();
	// Reregister the old frustrum
	PK_SESSION_register_frustrum ( &m_oldfrustrum );

}

// note this function does not clear the view
void CMeshWorkDoc::clear()
{
	m_ntopol = 0;
	m_ngeoms = 0;

	// clear selections
	if(m_pSelectionMgr->numPoints() > 0) m_pSelectionMgr->clear();
	if(m_picked.size() > 0) m_picked.clear();

	// delete the array of existing parts and geoms
	if( m_topols )
	{
		PK_MEMORY_free(m_topols);
		m_topols = NULL;
	}
	if( m_transfs )
	{
		PK_MEMORY_free(m_transfs);
		m_transfs = NULL;
	}
	if ( m_geoms )
	{
		PK_MEMORY_free(m_geoms);
		m_geoms = NULL;
	}
	if( m_pAsmMeshMgr )
	{
		delete m_pAsmMeshMgr;
		m_pAsmMeshMgr =  NULL;
	}
	if( m_pMeshCtrl )
	{
		delete m_pMeshCtrl;
		m_pMeshCtrl = NULL;
	}
	int nparts;
	PK_PART_t *parts;
	PK_SESSION_ask_parts( &nparts, &parts );
	if (nparts > 0)
	{
		PK_ENTITY_delete( nparts, parts );
		PK_MEMORY_free( parts );
	}

	int npartitions;
	PK_PARTITION_t	*partitions;
	PK_SESSION_ask_partitions( &npartitions, &partitions );
	ASSERT( npartitions == 1 );

	PK_PARTITION_ask_geoms( *partitions, &m_ngeoms, &m_geoms );
	if (m_ngeoms > 0)
	{
		PK_ENTITY_delete( m_ngeoms, m_geoms );
		PK_MEMORY_free( m_geoms );
	}
	PK_MEMORY_free( partitions );
}

// always start from scratch
void CMeshWorkDoc::setAssemblyMesh(pAssemblyMesh pNewAsmMesh)
{
	if( m_pAsmMeshMgr )
	{ m_pAsmMeshMgr->clear(); }
	else 
	{ m_pAsmMeshMgr = new AssemblyMeshMgr(); }
	m_pAsmMeshMgr->append(pNewAsmMesh);
	pNewAsmMesh->setID( m_pAsmMeshMgr->getFreeID() );

}

BOOL CMeshWorkDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// Register the graphics frustrum functions
	PK_SESSION_ask_frustrum( &m_oldfrustrum );
	m_frustrum = m_oldfrustrum;
	m_frustrum.goopsg = CopenSegment;
	m_frustrum.goclsg = CcloseSegment;
	m_frustrum.gosgmt = CoutputSegment;
	
	PK_SESSION_register_frustrum( &m_frustrum );

	return TRUE;
}

BOOL CMeshWorkDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	double dt1, dt2;
	CString strPathName(lpszPathName);
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	char *key = new char[strPathName.GetLength()+1];
	CStringToChar(key, strPathName);

	char dataFileName[255];
	strcpy_s(dataFileName, key);
	delete [] key;
	int length = (int) strlen(dataFileName);

	if(length>8 && (!strcmp(&dataFileName[length-8], ".xmt_txt") || !strcmp(&dataFileName[length-8], ".XMT_TXT")) ) {
		// Current view has to be the parasolid view   7/4/2012
		// otherwise opengl and osg are mixed up.
		if( m_view != mwApp->ActiveView() ) {
			AfxMessageBox(_T("Please switch view and open again!"),MB_OK);
			return FALSE;
		}
		if (!CDocument::OnOpenDocument(lpszPathName))
			return FALSE;
		dataFileName[length-8] = NULL;
		CString statusStr2;

		clock_t start2=clock();
		LoadPSModel(dataFileName);
		dt1 = (double)(clock()-start2)/CLOCKS_PER_SEC;
		statusStr2.Format(_T("Loaded and shown in %f seconds."),dt1);
		mwApp->SetStatusBarString(statusStr2);
	}
	else if(length>4 && (!strcmp(&dataFileName[length-4], ".bdf") || 
		                 !strcmp(&dataFileName[length-4], ".BDF") ||
						 !strcmp(&dataFileName[length-4], ".dat") ||
						 !strcmp(&dataFileName[length-4], ".DAT") ||
						 !strcmp(&dataFileName[length-4], ".pts") ||
						 !strcmp(&dataFileName[length-4], ".PTS") ||
						 !strcmp(&dataFileName[length-4], ".ibl")) ) {
		if( m_view == mwApp->ActiveView() ) {
			mwApp->SwitchView();
			//AfxMessageBox(_T("Please switch view and open again!"),MB_OK);
			//return FALSE;
		}
		if (!CDocument::OnOpenDocument(lpszPathName))
			return FALSE;

		//dataFileName[length-4] = NULL;
		int ret;
		CString statusStr =  _T("Loading ") + strPathName;
		mwApp->SetStatusBarString(statusStr);

		clock_t start=clock();
		if(!strcmp(&dataFileName[length-4], ".pts") || !strcmp(&dataFileName[length-4], ".PTS")) 
			ret = LoadPointCloud(dataFileName, true);
		else if(!strcmp(&dataFileName[length-4], ".ibl")) 
			ret = LoadIBL(dataFileName, true);
		else
			ret = LoadBDF(dataFileName, true);
		if( ret == 0 ) {
			dt1 = (double)(clock()-start)/CLOCKS_PER_SEC;
			statusStr.Format(_T("Loaded in %f seconds."),dt1);
			mwApp->SetStatusBarString(statusStr);

			// create scene graph
			CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
			mwApp->sysOption()->setSceneStyle(Facets);	// always using Facets style, otherwise normal array will be an issue
		
			clock_t start=clock();
			meView->SetSceneData(0,true);
		    dt2 = (double)(clock()-start)/CLOCKS_PER_SEC;
			statusStr.Format(_T("Loaded in %f seconds. Set scene in %f seconds."),dt1, dt2);
			mwApp->SetStatusBarString(statusStr);
		}

	}
	else {
		AfxMessageBox(_T("Please select a .xmt_txt file, .ibl file, .pts file, .bdf or .dat file!"),MB_OK);
		return FALSE;
	}

	//TCHAR Buffer[MAX_PATH];
	//DWORD dwRet = GetCurrentDirectory(MAX_PATH, Buffer);
	SetCurrentDirectory(curworkingdir);
	return TRUE;
}

void CMeshWorkDoc::OnCloseDocument()
{
	// TODO: Add your command handler code here
	//exit(0);
	//TRACE(_T("enter OnCloseDocument.\n"));
	CDocument::OnCloseDocument();
}											// deleaker

// retrieve and display parasolid parts
void CMeshWorkDoc::LoadPSModel(char dataFileName[255]) 
{
	// 1. clear all data for previous model
	clear();

	// 2. load the part into current partition
	int nparts = 0;
	PK_PART_t *parts = NULL;
	if( receive_parts(dataFileName,&nparts,&parts) ) {
		return;
	}

	// 3. flatten assemblies
	PK_CLASS_t eclass;
	vector<PK_BODY_t> bodies;
	vector<PK_TRANSF_t> transforms;
	BOOL hasAssembly = FALSE;
	for(int i=0; i<nparts; i++) {
		PK_ENTITY_ask_class( parts[i], &eclass );
		if(eclass == PK_CLASS_assembly) {
			assembly_ask_bodies_transfs(parts[i],NULL,bodies,transforms);
			hasAssembly = TRUE;
		}
		else if(eclass == PK_CLASS_body) {
			PK_BODY_type_t body_type;
			PK_BODY_ask_type(parts[i],&body_type);
			if(body_type == PK_BODY_type_solid_c || 
			   body_type == PK_BODY_type_general_c ||
			   body_type == PK_BODY_type_sheet_c) {
				   bodies.push_back(parts[i]);
				   transforms.push_back(NULL);
			}
		}
	}
	PK_MEMORY_free(parts);
	// vector -> array
	m_topols = NULL;
	m_transfs = NULL;
	m_ntopol = bodies.size();
	PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
	std::copy(bodies.begin(),bodies.end(),m_topols);
	if(hasAssembly) {
		PK_MEMORY_alloc( sizeof( PK_TRANSF_t ) * m_ntopol, (void**)&m_transfs );
		std::copy(transforms.begin(),transforms.end(),m_transfs);
	} 

	// 4. update the parasolid view
	UpdatePSView();
}

void CMeshWorkDoc::UpdatePSView()
{
	// Get all the geometry in the partition. 
	PK_ERROR_code_t ret;
	int npartitions;
	PK_PARTITION_t	*partitions = NULL;
	ret = PK_SESSION_ask_partitions( &npartitions, &partitions );
	ASSERT( npartitions == 1 );
	ret = PK_PARTITION_ask_geoms( partitions[0], &m_ngeoms, &m_geoms );
	PK_MEMORY_free( partitions );

	// view the part
	m_view->OnUpdate(NULL,NULL,NULL);
	m_view->FitPartsInView();   
}

double stringToVal(std::string str)
{
	std::string::iterator it = str.begin();
	for( ; it!=str.end(); ) {
		if((*it) == ' ')
			++it;
		else
			break;
	}
	++it;
	for( ; it!=str.end(); ++it) {
		if((*it) == '-')
			break;
	}
	if( it != str.end() ) {
		if( isdigit(*(--it)) ) {
			++it;
			str.insert(it,1,'e');
		}
	}
	return atof(str.c_str());
}

// retrieve and display nastran bdf
int CMeshWorkDoc::LoadBDF(char dataFileName[255], bool clear_exist) 
{
  FILE *bdf_fp;
  bdf_fp = fopen(dataFileName, "r");
  if (bdf_fp == NULL) {
    return 1;
  }
    
  char line[MAX_LINE], str[MAX_LINE];
  std::string sstr;	

  if(clear_exist || !m_pAsmMeshMgr) {
	  // clean everything
	  clear();
	  m_view->OnUpdate(NULL,NULL,NULL);
	  m_pAsmMeshMgr = new AssemblyMeshMgr();
  }

  // load BDF into m_pAsmMesh
  pAssemblyMesh m_pAsmMesh = new AssemblyMesh();
  pMWMesh me = m_pAsmMesh->ithInstancedMesh(0);
  m_pAsmMesh->setID( m_pAsmMeshMgr->getFreeID() );

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath(dataFileName,drive,dir,fname,ext);
  m_pAsmMesh->setName(fname);
 
  m_pAsmMeshMgr->setCurrent(m_pAsmMeshMgr->size());  // must before append
  m_pAsmMeshMgr->append(m_pAsmMesh);

  //Implement BDF file reader
  //Go to the top of the file
  rewind( bdf_fp ) ;

  //Loop through the cards and scan for GRID* or GRID
  std::map<int,pVertex> id2vertex;  // build id -> vertex map
  pVertex vt;
  int id, nFound;
  double xyz[3];
  int numGRID = 0;
  while( fgets( line, MAX_LINE, bdf_fp ) ) {
	if(strlen(line) < 8)
		continue;
    sscanf( line, "%s", str ) ;
    if( strncmp( str, "GRID", 4 ) != 0 ) 
      continue ;
    if( strcmp( str, "GRID*" ) == 0 ) {
      // sscanf( line, "%s %d %le %le", str, &(id), &(xyz[0]), &(xyz[1]) ) ;
	  sstr.assign(line);
	  // 8 16 16 16 16
	  id = atoi( (sstr.substr(8,16)).c_str() );
	  xyz[0] = stringToVal( sstr.substr(40,16) );
	  xyz[1] = stringToVal( sstr.substr(56,16) );
      fgets( line, MAX_LINE, bdf_fp );
	  //sscanf( line, "%s %le", str, &(xyz[2]) ) ;
	  sstr.assign(line);
	  xyz[2] = stringToVal( sstr.substr(8,16) );
      
    } else if ( strcmp( str, "GRID" ) == 0 ) {
	  //char str1[5][9];
	  //sscanf(strrev(line),"%8s%8s%8s%8s%8s",str1[4],str1[3],str1[2],str1[1],str1[0]);
	  //strcpy(str, strrev(str1[0]));
	  //id = atoi( strrev(str1[1]) );
	  //xyz[0] = atof( strrev(str1[2]) );
	  //xyz[1] = atof( strrev(str1[3]) );
	  //xyz[2] = atof( strrev(str1[4]) );	
	  sstr.assign(line);
	  id = atoi( (sstr.substr(8,8)).c_str() );
	  //if( id == 4985 ) {
		 //std::string sub_str1 = sstr.substr(24,8);
		 //std::string sub_str2 = sstr.substr(32,8);
		 //std::string sub_str3 = sstr.substr(40,8);
		 //double val = stringToVal(sub_str1);
		 // val = stringToVal("-0.2323");
		 // val = stringToVal("1.2323e6");
		 // val = stringToVal("1.2323e-2");
		 // val = stringToVal("1.2323E6");
	  //}
	  xyz[0] = stringToVal( sstr.substr(24,8) );
	  xyz[1] = stringToVal( sstr.substr(32,8) );
	  xyz[2] = stringToVal( sstr.substr(40,8) );

	} else if ( strcmp( str, "GRID," ) == 0 ) {
		// comma seperated values
		nFound = sscanf(line, "%s %d%*c %*c %le%*c %le%*c %le", str, &(id), &(xyz[0]), &(xyz[1]), &(xyz[2]) );

    } else {
      continue ;
    }

	//if(id == 115641 || id == 120399 )
	//	id = id;

    //create mesh vertex in PS mesh's representaion with NULL classification
	vt = M_createVertex(me,xyz,NULL);
	EN_resetID((pEntity)vt,id);
	id2vertex[id] = vt;
	numGRID++;
  }

  if(numGRID < 1) {
	  fclose(bdf_fp);
	  AfxMessageBox(_T("no nodes in given BDF file!"),MB_OK);
	  return 2;
  }

  pRegion rgn;
  pFace fc;
  pEdge ed;
  pVertex verts[4];
  BDF_FORMAT format;

  int elem_id, prop_id, num, n[6], freestyle;
  char elem_str[MAX_LINE];
  MGCElemType type;

  //Go to the top of the file
  rewind( bdf_fp ) ;

  //Loop through the cards and scan for CMASS1, CBAR, CTRIA3, CTRIA6
  //   CQUAD4, CQUAD8, CTETRA, CPENTA and CHEXA
  while( fgets( line, MAX_LINE, bdf_fp ) ) {

	if(strlen(line) < 8)
		continue;
    if( strncmp( line, "C", 1 ) != 0 ) 
      continue ;
	sscanf( line, "%s", str ) ;

    format = short_format ;
	freestyle = 0;
    if ( (strcmp( str, "CTETRA" ) == 0) || (strcmp( str, "CTETRA*" ) == 0) ){
      num = 4;
      type = MGC_ETYPE_TET4;
      if( str[6] == '*' ) format = long_format ;
    }
    else if ( strncmp(str,"CTRIA3",6) == 0 || strncmp(str,"CTRIA6",6) == 0){
      num = 3;
      type = MGC_ETYPE_TRIA3;
      if( str[6] == '*' ) format = long_format ;
	  if( str[6] == ',' ) freestyle = 1 ;
	} else if ( strncmp( str, "CQUAD", 5 ) == 0 ){
      if( str[6] == '*' ) format = long_format ;
	  if( str[6] == ',' ||  str[5] == ',') freestyle = 1 ;
	  if( str[5] == '8' ) 
	  { num=8; type = MGC_ETYPE_QUAD8; }
	  else 
	  { num=4; type = MGC_ETYPE_QUAD4; }
	} else if ( (strcmp( str, "CBAR" ) == 0) || (strcmp( str, "CBAR*" ) == 0) ){
	  num = 2;
	  type = MGC_ETYPE_BAR2;
      if( str[4] == '*' ) format = long_format ;
	} else if ( (strcmp( str, "CMASS1" ) == 0) || (strcmp( str, "CMASS1*" ) == 0) ){
	  num = 1;
	  type = MGC_ETYPE_POINT;
      if( str[4] == '*' ) format = long_format ;
    } else {
      continue ;
    }

    if (type == MGC_ETYPE_TET4) {
      //Get the nodes of the element
      if (format == short_format) {
		sscanf( line, "%s %d %d %d %d %d %d", 
				elem_str, &elem_id, &prop_id,
				&(n[0]), &(n[1]), &(n[2]), &(n[3]) ) ;
      }
      else if (format == long_format) {
        //Not implemented
		AfxMessageBox(_T("Detected tet4 element in long format. Not implemented!"),MB_OK);
      }
	  verts[0] = id2vertex[n[0]]; // vts[ n[0] - 1 ];
	  verts[1] = id2vertex[n[1]]; // vts[ n[1] - 1 ];
	  verts[2] = id2vertex[n[2]]; // vts[ n[2] - 1 ];
	  verts[3] = id2vertex[n[3]]; // vts[ n[3] - 1 ];
	  if(!verts[0] || !verts[1] || !verts[2] || !verts[3]) {
		  CString cstr;
		  cstr.Format(_T("CTETRA Node %d %d %d or %d do not exist!"), n[0], n[1], n[2], n[3]);
		  AfxMessageBox(cstr,MB_OK);
		  break;
	  }
	  rgn = M_createRegion(me,verts,NULL);
	  EN_resetID((pEntity)rgn,elem_id);
    }
    else if (type == MGC_ETYPE_TRIA3) {
      //Get the nodes of the face
	  if (freestyle) {
		  nFound = sscanf(line, "%s %d%*c %d%*c %d%*c %d%*c %d", elem_str, &(elem_id), &(prop_id), &(n[0]), &(n[1]), &(n[2]) ) ;
	  } 
	  else if (format == short_format) {
		//nFound = sscanf( line, "%s %d %d %d %d %d", 
		//				elem_str, &elem_id, &prop_id,
		//				&(n[0]), &(n[1]), &(n[2]) ) ;
		//if(nFound < 6) {
		//	// simX may not write prop_id
		//	n[2] = n[1];
		//	n[1] = n[0];
		//	n[0] = prop_id;
		//	prop_id = 0;
		//} 
		  
		sstr.assign(line);  // changed to load h4m_2000Hz_linear.bdf
		elem_id = atoi( (sstr.substr(8,8)).c_str() );
		//prop_id = atoi( (sstr.substr(16,8)).c_str() );
		n[0] = atoi( (sstr.substr(24,8)).c_str() );
		n[1] = atoi( (sstr.substr(32,8)).c_str() );
		n[2] = atoi( (sstr.substr(40,8)).c_str() );
		
      }
      else if (format == long_format) {
		sscanf( line, "%s %d %d %d %d", elem_str, &(elem_id), &(prop_id), &(n[0]), &(n[1]) ) ;
        fgets( line, MAX_LINE, bdf_fp );
		sscanf( line, "%s %d", elem_str, &(n[2]) ) ;
		// AfxMessageBox(_T("Detected tri3 element in long format. Not implemented!"),MB_OK);
      }
	  verts[0] = id2vertex[n[0]]; // vts[ n[0] - 1 ];
	  verts[1] = id2vertex[n[1]]; // vts[ n[1] - 1 ];
	  verts[2] = id2vertex[n[2]]; // vts[ n[2] - 1 ];
	  if(!verts[0] || !verts[1] || !verts[2]) {
		  CString cstr;
		  cstr.Format(_T("CTRIA3 Node %d %d or %d do not exist!"), n[0], n[1], n[2]);
		  AfxMessageBox(cstr,MB_OK);
		  break;
	  }
	  //fc = M_createFace(me,3,verts,NULL);
	  fc = M_createFace(me,3,verts,prop_id);
	  EN_resetID((pEntity)fc,elem_id);
    }
	else if (type == MGC_ETYPE_QUAD4) {
	  //Get the nodes of the face
	  if (freestyle) {
		  nFound = sscanf(line, "%s %d%*c %d%*c %d%*c %d%*c %d%*c %d", elem_str, &(elem_id), &(prop_id), &(n[0]), &(n[1]), &(n[2]), &(n[3]) ) ;
	  } 
      else if (format == short_format) {
		nFound = sscanf( line, "%s %d %d %d %d %d %d", 
						elem_str, &elem_id, &prop_id,
						&(n[0]), &(n[1]), &(n[2]), &(n[3]) ) ;
		if(nFound < 7) {
			// simX may not write prop_id
			n[3] = n[2];
			n[2] = n[1];
			n[1] = n[0];
			n[0] = prop_id;
			prop_id = 0;
		}
      }
      else if (format == long_format) {
		sscanf( line, "%s %d %d %d %d", elem_str, &(elem_id), &(prop_id), &(n[0]), &(n[1]) ) ;
        fgets( line, MAX_LINE, bdf_fp );
		sscanf( line, "%s %d %d", elem_str, &(n[2]), &(n[3]) ) ;
      }
	  verts[0] = id2vertex[n[0]]; // vts[ n[0] - 1 ];
	  verts[1] = id2vertex[n[1]]; // vts[ n[1] - 1 ];
	  verts[2] = id2vertex[n[2]]; // vts[ n[2] - 1 ];
	  verts[3] = id2vertex[n[3]]; 
	  if(!verts[0] || !verts[1] || !verts[2] || !verts[3]) {
		  CString cstr;
		  cstr.Format(_T("CQUAD Node %d %d %d or %d do not exist!"), n[0], n[1], n[2], n[3]);
		  AfxMessageBox(cstr,MB_OK);
		  break;
	  }
	  fc = M_createFace(me,4,verts,NULL);
	  EN_resetID((pEntity)fc,elem_id);
	} 
	else if (type == MGC_ETYPE_QUAD8) {  // read as CQUAD4
	  //Get the nodes of the face
      if (format == short_format) {
		nFound = sscanf( line, "%s %d %d %d %d %d %d %d %d", 
						elem_str, &elem_id, &prop_id,
						&(n[0]), &(n[1]), &(n[2]), &(n[3]), &(n[4]), &(n[5]) ) ;
		fgets( line, MAX_LINE, bdf_fp );
		if(nFound < 8) {
			// simX may not write prop_id
			n[3] = n[2];
			n[2] = n[1];
			n[1] = n[0];
			n[0] = prop_id;
			prop_id = 0;
		}
      }
      else if (format == long_format) {
		AfxMessageBox(_T("Detected CQUAD8 in long format. Not implemented!"),MB_OK);
      }
	  verts[0] = id2vertex[n[0]]; // vts[ n[0] - 1 ];
	  verts[1] = id2vertex[n[1]]; // vts[ n[1] - 1 ];
	  verts[2] = id2vertex[n[2]]; // vts[ n[2] - 1 ];
	  verts[3] = id2vertex[n[3]]; 
	  if(!verts[0] || !verts[1] || !verts[2] || !verts[3]) {
		  CString cstr;
		  cstr.Format(_T("CQUAD Node %d %d %d or %d do not exist!"), n[0], n[1], n[2], n[3]);
		  AfxMessageBox(cstr,MB_OK);
		  break;
	  }
	  fc = M_createFace(me,4,verts,NULL);
	  EN_resetID((pEntity)fc,elem_id);
	} 
	else if (type == MGC_ETYPE_BAR2) {
      if (format == short_format) {
		  sstr.assign(line);  
		  elem_id = atoi( (sstr.substr(8,8)).c_str() );
		  //prop_id = atoi( (sstr.substr(16,8)).c_str() );
		  n[0] = atoi( (sstr.substr(24,8)).c_str() );
		  n[1] = atoi( (sstr.substr(32,8)).c_str() );

      }
      else if (format == long_format) {
		sscanf( line, "%s %d %d %d %d", elem_str, &(elem_id), &(prop_id), &(n[0]), &(n[1]) ) ;
        fgets( line, MAX_LINE, bdf_fp );
		sscanf( line, "%s %le %le %le", elem_str, &(xyz[0]), &(xyz[1]), &(xyz[2]) ) ;
		//Not implemented
		//AfxMessageBox(_T("Detected BAR2 element in long format. Not implemented!"),MB_OK);
      }
	  verts[0] = id2vertex[n[0]]; // vts[ n[0] - 1 ];
	  verts[1] = id2vertex[n[1]]; // vts[ n[1] - 1 ];
	  if(!verts[0] || !verts[1]) {
		  CString cstr;
		  cstr.Format(_T("CBAR Node %d or %d do not exist!"), n[0], n[1]);
		  AfxMessageBox(cstr,MB_OK);
		  break;
	  }
	  ed = M_createEdge(me,verts[0],verts[1],NULL);
	  EN_resetID((pEntity)ed,elem_id);
	}
	else if (type == MGC_ETYPE_POINT) {
	  if (format == short_format) {
		  sstr.assign(line);  
		  elem_id = atoi( (sstr.substr(8,8)).c_str() );
		  //prop_id = atoi( (sstr.substr(16,8)).c_str() );
		  n[0] = atoi( (sstr.substr(24,8)).c_str() );
      }
      else if (format == long_format) {
		  AfxMessageBox(_T("Detected CMASS element in long format. Not implemented!"),MB_OK);
		  break;
	  }
	  verts[0] = id2vertex[n[0]];
	  V_resetClassificationType(verts[0],0);
	}
  }
  fclose(bdf_fp);

  return 0;
}

// retrieve and display point clouds
int CMeshWorkDoc::LoadPointCloud(char dataFileName[255], bool clear_exist) 
{
  FILE *cld_fp;
  cld_fp = fopen(dataFileName, "r");
  if (cld_fp == NULL) {
    return 1;
  }

  char line[MAX_LINE], str[MAX_LINE], str2[MAX_LINE];

  // 1. clean everything
  if(clear_exist || !m_pAsmMeshMgr) {
	  // clean everything
	  clear();
	  m_view->OnUpdate(NULL,NULL,NULL);
	  m_pAsmMeshMgr = new AssemblyMeshMgr();
  } 

  // 2. load point cloud into m_pAsmMesh
  pAssemblyMesh m_pAsmMesh = new AssemblyMesh();	
  pMWMesh me = m_pAsmMesh->ithInstancedMesh(0);
  m_pAsmMesh->setID( m_pAsmMeshMgr->getFreeID() );
  m_pAsmMeshMgr->setCurrent(m_pAsmMeshMgr->size()); // must before append
  m_pAsmMeshMgr->append(m_pAsmMesh);

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME], pcfname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath(dataFileName,drive,dir,fname,ext);
  sprintf(pcfname,"%s_pointcloud",fname);
  m_pAsmMesh->setName(pcfname);

  //Go to the top of the file
  rewind( cld_fp ) ;

  double scale = 1.0;
  int i=0;
  while( fgets( line, MAX_LINE, cld_fp ) ) {
	if(strlen(line) < 8)
		continue;
    sscanf( line, "%s", str ) ;
	if( strncmp( str, "#unit", 5 ) == 0 ) {
		sscanf(line, "%s %s", str, str2 );
		if( strncmp( str2, "mm", 2 ) == 0 ) 
			scale = 0.001;
		if( strncmp( str2, "inch", 2 ) == 0 ) 
			scale = 0.0254;
	} 
  }

  rewind( cld_fp ) ;

  //Read points
  pVertex vt;
  int id=1, nFound, nSkip=0;
  double xyz[3];
  while( fgets( line, MAX_LINE, cld_fp ) ) {
	if(strlen(line) < 8)
		continue;
    sscanf( line, "%s", str ) ;
	if( strncmp( str, "#", 1 ) == 0 ) 
      continue ;

	// space seperated values
	nFound = sscanf(line, "%le %le %le", &(xyz[0]), &(xyz[1]), &(xyz[2]) );
    if(nFound != 3) {
		// check comma seperated values
		nFound = sscanf(line, "%le%*c %le%*c %le", &(xyz[0]), &(xyz[1]), &(xyz[2]) );
		if(nFound != 3)
		{ nSkip++; continue ; }
	}

    //create mesh vertex in PS mesh's representaion with NULL classification
	if(scale != 1.0)
	{ xyz[0] *= scale; xyz[1] *= scale; xyz[2] *= scale; }
	vt = M_createVertex(me,xyz,NULL);
	EN_resetID((pEntity)vt,id);
	id++;
  }

  fclose(cld_fp);

  if(id == 1) {
	  AfxMessageBox(_T("no cloud point is read!"),MB_OK);
	  return 2;
  }
  if(nSkip > 0) {
	  CString cstr;
	  cstr.Format(_T("Skipped %d unknown lines!"), nSkip);
	  AfxMessageBox(cstr,MB_OK);
  }
  return 0;
}

// proE curves
int CMeshWorkDoc::LoadIBL(char dataFileName[255], bool clear_exist) 
{
  FILE *fp = fopen(dataFileName, "r");
  if (fp == NULL) 
    return 1;

  char line[MAX_LINE], str[MAX_LINE], str2[MAX_LINE];
  char name[64], str3[64], str4[64], str5[64];

  // 1. clean everything
  if(clear_exist || !m_pAsmMeshMgr) {
	  // clean everything
	  clear();
	  m_view->OnUpdate(NULL,NULL,NULL);
	  m_pAsmMeshMgr = new AssemblyMeshMgr();
  } 

  // 2. load point cloud into m_pAsmMesh
  pAssemblyMesh m_pAsmMesh = new AssemblyMesh();	
  pMWMesh me = m_pAsmMesh->ithInstancedMesh(0);
  m_pAsmMesh->setID( m_pAsmMeshMgr->getFreeID() );
  m_pAsmMeshMgr->setCurrent(m_pAsmMeshMgr->size()); // must before append
  m_pAsmMeshMgr->append(m_pAsmMesh);

  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME], iblfname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath(dataFileName,drive,dir,fname,ext);
  sprintf(iblfname,"%s_ibl",fname);
  m_pAsmMesh->setName(iblfname);

  int sectionID = 0;
  int curveID = 0;
  int i, nFound;

  //Go to the top of the file
  rewind( fp ) ;

  //Read points
  while( fgets( line, MAX_LINE, fp ) ) {
	if(strlen(line) < 8)
		continue;
    nFound = sscanf( line, "%s", str) ;
	if( strncmp( str, "#", 1 ) == 0 ) 
      continue ;

	if(strncmp( str, "begin", 5 ) == 0 ) {
		nFound = sscanf( line, "%s %s %s %s %s", str,str2,str3,str4,str5) ;
		if( strncmp( str2, "section", 7 ) == 0 ) {
			sectionID++;
			if(nFound >= 5)	sprintf(name,"%s_%s",str4,str5);
			else			sprintf(name,"%s",str4);
			continue;
		}

		if( strcmp( str2, "curve" ) == 0 ) {
			std::vector<node_t> crv;
			node_t pt;
			curveID++;
			pt.id=0; pt.h=0.0;
			while( fgets( line, MAX_LINE, fp ) ) {
				nFound = sscanf(line, "%le %le %le", &(pt.x), &(pt.y), &(pt.z) );
				if(nFound < 3)
					break;
				crv.push_back(pt);
			}
			// add the curve into MWMesh
			int np = crv.size();
			if(np > 1) {
				double *pts = new double[np*3];
				std::vector<node_t>::iterator itr;
				i=0;
				for(itr=crv.begin(); itr!=crv.end(); itr++) {
					pt = *itr;
					pts[i++] = pt.x;
					pts[i++] = pt.y;
					pts[i++] = pt.z;
				}
				GM_createPolyLine(me,name,np,pts);
				delete [] pts;
			}
		}
	}
  }

  fclose(fp);

  if(curveID == 0) {
	  AfxMessageBox(_T("no curve read!"),MB_OK);
	  return 2;
  }
  //CString cstr;
  //cstr.Format(_T("Read %d sections and %d curves!"), sectionID, curveID);
  //AfxMessageBox(cstr,MB_OK);

  GM_createBlade(me);
  return 0;
}

int CMeshWorkDoc::receive_parts(char dataFileName[255], int *n_parts, PK_PART_t** parts)
{
	char key[255];
	sprintf(key,"%s",dataFileName);
	CString str(key);
	CString str1, str2, str3;
	str1 = _T("Receiving ") + str;
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	mwApp->SetStatusBarString(str1);

	PK_PART_receive_o_t receive_opts;
	PK_PART_receive_o_m( receive_opts );
    receive_opts.transmit_format = PK_transmit_format_text_c;
    int ifail = PK_PART_receive( dataFileName, &receive_opts, n_parts, parts ); 
	if(ifail==KI_key_not_found)
	{
		str3 = _T("Can't open ") + str;
		mwApp->SetStatusBarString(str3);
	}
	else if( ifail != PK_ERROR_no_errors || n_parts <= 0) { 
		if (getenv("P_SCHEMA")) {
			CString str4(getenv("P_SCHEMA"));
			str3 = _T("PARASOLID environment variable is ") + str4;
		}
		else {
			str3 = _T("PARASOLID environment variable is NOT SET!");
		}
		mwApp->SetStatusBarString(str3);
	}

 	if (*n_parts == 0)
		return 1;

	if( *n_parts == 1 )
		str3.Format(_T(" (%d part)"),*n_parts);
	else
		str3.Format(_T(" (%d parts)"),*n_parts);
	str2 = _T("Received ") + str + str3;
	mwApp->SetStatusBarString(str2);

	return 0;
}

void CMeshWorkDoc::assembly_ask_bodies_transfs(PK_ASSEMBLY_t assem, PK_TRANSF_t transf_in, vector<PK_BODY_t> &bodies, vector<PK_TRANSF_t> &transfs_out)
{
	PK_CLASS_t pclass;
	PK_ERROR_code_t ret;

	int n_parts;
	PK_PART_t *parts;
	PK_TRANSF_t *transfs;
	PK_ASSEMBLY_ask_parts_transfs(assem,&n_parts,&parts,&transfs);

	for( int i=0; i<n_parts; i++ ) {
		if( !parts[i] ) continue;
		// propogate transforms
		PK_TRANSF_t transf_combined;
		if(transf_in != NULL)
			ret = PK_TRANSF_transform(transfs[i],transf_in,&transf_combined);

		PK_ENTITY_ask_class(parts[i], &pclass);
		if( pclass == PK_CLASS_assembly ) {
			printf("Assembly");
			if(transf_in == NULL)
				assembly_ask_bodies_transfs(parts[i],transfs[i],bodies,transfs_out);
			else
				assembly_ask_bodies_transfs(parts[i],transf_combined,bodies,transfs_out);
		} 
		else if( pclass == PK_CLASS_body ) {  
			PK_BODY_type_t body_type;
			PK_BODY_ask_type(parts[i],&body_type);
			if(body_type == PK_BODY_type_solid_c || 
			   body_type == PK_BODY_type_general_c ||
			   body_type == PK_BODY_type_sheet_c) {			
				   bodies.push_back(parts[i]);
				   printf("push_back: %d  body=%d\n",bodies.size(), parts[i]);
				   if(transf_in == NULL)
					   transfs_out.push_back(transfs[i]);
				   else 
					   transfs_out.push_back(transf_combined);
			}
		}
	}
	return;
}

void CMeshWorkDoc::AddPSModel(int mo_index)
{
	double pi = 3.14159265358979323846;
	double f= 0.001;

	clear();
	m_topols = NULL;
	m_transfs = NULL;

	//mo_index = m_psModelIndex%3 + 1;
	//m_psModelIndex++;
	if(mo_index == 1) {
		PK_BODY_t block, cylinder, cone, sphere;
		PK_AXIS2_sf_t basis_set;
		PK_BODY_create_solid_block( 10.0*f, 10.0*f, 2.0*f, NULL, &block );

		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = 1*f;
		basis_set.axis.coord[0] = 0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 1;
		basis_set.ref_direction.coord[0] = 0;
		basis_set.ref_direction.coord[1] = 1;
		basis_set.ref_direction.coord[2] = 0;
		PK_BODY_create_solid_cyl( 0.5*f, 20.0*f, &basis_set, &cylinder );

		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = -2.5*f;
		PK_BODY_create_solid_sphere( 2.5*f, &basis_set, &sphere );

		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = 25*f;
		basis_set.axis.coord[0] = 0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = -1;
		PK_BODY_create_solid_cone( 0.0, 4.0*f, pi/12.0, &basis_set, &cone );

		m_ntopol = 4;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
		m_topols[0] = block;
		m_topols[1] = cylinder;
		m_topols[2] = cone;
		m_topols[3] = sphere;
	}

	if(mo_index == 2) {
		PK_BODY_t block1, block2;
		PK_AXIS2_sf_t basis_set;
		PK_BODY_create_solid_block( 10.0, 10.0, 2.0, NULL, &block1 );

		basis_set.location.coord[0] = -5;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = -4;
		basis_set.axis.coord[0] = 1;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 0;
		basis_set.ref_direction.coord[0] = 0;
		basis_set.ref_direction.coord[1] = 1;
		basis_set.ref_direction.coord[2] = 0;
		PK_BODY_create_solid_block( 10.0, 10.0, 2.0, &basis_set, &block2 );

		m_ntopol = 2;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
		m_topols[0] = block1;
		m_topols[1] = block2;
	}
	if(mo_index == 3) {
		PK_BODY_t block, cylinder;
		PK_AXIS2_sf_t basis_set;
		PK_BODY_boolean_o_t opts;
	    PK_TOPOL_track_r_t      tracking;
	    PK_boolean_r_t          results;    

		PK_BODY_create_solid_block( 10.0, 10.0, 10.0, NULL, &block );  
 
		basis_set.location.coord[0] = 0;
		basis_set.location.coord[1] = 0;
		basis_set.location.coord[2] = -5;
		basis_set.axis.coord[0] = 0;
		basis_set.axis.coord[1] = 0;
		basis_set.axis.coord[2] = 1;
		basis_set.ref_direction.coord[0] = 1;
		basis_set.ref_direction.coord[1] = 0;
		basis_set.ref_direction.coord[2] = 0;
		PK_BODY_create_solid_cyl( 2.5, 20.0, &basis_set, &cylinder );

		PK_BODY_boolean_o_m( opts );
		PK_BODY_boolean_2( block, 1, &cylinder, &opts, &tracking, &results );
		PK_TOPOL_track_r_f(&tracking);
		PK_boolean_r_f(&results );    

		m_ntopol = 1;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
		m_topols[0] = block;
	}

	UpdatePSView();
}

void CMeshWorkDoc::InsertPSphere4P()
{
	double xyz[10][3], center[3], r;
	int i;

	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();
	int np = getSelectionMgr()->numPoints();
	if(np < 4) {
		mwApp->SetStatusBarString("Please pick four points, and click Geometry/Insert/Sphere...");
		return;
	}
	if(np > 4){
		mwApp->SetStatusBarString("Please clear point selections, then pick four points and click Geometry/Insert/Sphere...");
		return;
	}

	for(i=0; i<np; i++) {
		MW_Point pt=getSelectionMgr()->ithPoint(i);
		xyz[i][0] = pt.x();
		xyz[i][1] = pt.y();
		xyz[i][2] = pt.z();
	}

	int err = XYZ_circumSphere(xyz, center, &r);
	if(err) {
		mwApp->SetStatusBarString("Volume is too small. Inserting Parasolid sphere failed.");
		return;
	}

	PK_BODY_t sphere;
	//PK_AXIS2_sf_t basis_set;

	//basis_set.location.coord[0] = center[0];
	//basis_set.location.coord[1] = center[1];
	//basis_set.location.coord[2] = center[2];
	PK_ERROR_t pkerr = PK_BODY_create_solid_sphere( r, NULL, &sphere );

	if(m_ntopol == 0) {
		m_ntopol = 1;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
		m_topols[0] = sphere;
	} else {
		PK_TOPOL_t* m_topols_bk;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols_bk);
		for(i=0; i<m_ntopol; i++)
			m_topols_bk[i] = m_topols[i];
		m_ntopol += 1;
		PK_MEMORY_alloc( sizeof( PK_TOPOL_t ) * m_ntopol, (void**)&m_topols );
		for(i=0; i<m_ntopol-1; i++)
			m_topols[i] = m_topols_bk[i];
		m_topols[m_ntopol-1] = sphere;
		PK_MEMORY_free(m_topols_bk);
	}

	mwApp->SwitchView();
	UpdatePSView();
}

char* CStringToChar(char* lpszStr, CString& strText)
{
USES_CONVERSION;
	strcpy( lpszStr, T2A((LPTSTR)(LPCTSTR) strText ));

	return lpszStr;
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkDoc serialization

void CMeshWorkDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkDoc diagnostics

#ifdef _DEBUG
void CMeshWorkDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMeshWorkDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkDoc commands

void CMeshWorkDoc::OnFileInsert()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	// TODO: Add your command handler code here
	if(!m_pAsmMeshMgr) {
		mwApp->SetStatusBarString(_T("Empty mesh document. You have to first open/generate a mesh..."));
		return;
	}

	char strFilter[] = { _T("NASTRAN (*.bdf)|*.bdf|PointCloud (*.pts)|*.pts|") };
	//wchar_t strFilter[] = { _T("NASTRAN (*.bdf)|*.bdf|PointCloud (*.pts)|*.pts|") };
	CFileDialog dlg(TRUE,NULL,NULL,OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST,strFilter);
	dlg.m_ofn.lpstrTitle = _T("Insert");
	if (dlg.DoModal() != IDOK)
		return;

	CString pathName = dlg.GetPathName();
	CString fileExt = dlg.GetFileExt();
	CString statusStr =  _T("Inserting ") + pathName;
	mwApp->SetStatusBarString(statusStr);
	char *key = new char[pathName.GetLength()+1];
	CStringToChar(key, pathName);

	CString str;
	double dt1, dt2;
	clock_t start=clock();

	if(fileExt == "bdf")
	{
		LoadBDF(key, false);
	}
	else if(fileExt == "pts")
	{
		LoadPointCloud(key, false);
	}

	dt1 = (double)(clock()-start)/CLOCKS_PER_SEC;
	statusStr.Format(_T("Loaded in %f seconds."),dt1);
	mwApp->SetStatusBarString(statusStr);
	delete [] key;

	// update scene graph
	CMeshWorkOsgView* meView = (CMeshWorkOsgView *) mwApp->MeshView();
	mwApp->sysOption()->setSceneStyle(Facets);	// always using Facets style, otherwise normal array will be an issue
		
	start=clock();
	meView->SetSceneData(1,false);
	dt2 = (double)(clock()-start)/CLOCKS_PER_SEC;
	statusStr.Format(_T("Loaded in %f seconds. update scene in %f seconds."),dt1, dt2);
	mwApp->SetStatusBarString(statusStr);

	return;
}

void CMeshWorkDoc::OnFileSaveas()
{
	CMeshWorks* mwApp = (CMeshWorks*)AfxGetApp();

	// TODO: Add your command handler code here
	if(!m_pAsmMeshMgr) {
		mwApp->SetStatusBarString("No mesh to save.");
		return;
	}
	pAssemblyMesh m_pAsmMesh = m_pAsmMeshMgr->ith(m_pAsmMeshMgr->current());
	if(!m_pAsmMesh) {
		mwApp->SetStatusBarString("No mesh to save.");
		return;
	}

	// get file name and extension
	char strFilter[] = { _T("osgviewer (*.osg)|*.osg|NASTRAN (*.bdf)|*.bdf|medit (*.mesh)|*.mesh|") };
	//wchar_t strFilter[] = { _T("osgviewer (*.osg)|*.osg|NASTRAN (*.bdf)|*.bdf|medit (*.mesh)|*.mesh|") };
	CFileDialog dlg(FALSE,NULL,NULL,OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST,strFilter);
	if (dlg.DoModal() != IDOK)
		return;
	
	CString pathName = dlg.GetPathName();
	CString fileExt = dlg.GetFileExt();
	CString statusStr =  _T("Saved mesh to ") + pathName;
	CString str;
	char *key = new char[pathName.GetLength()+1];
	CStringToChar(key, pathName);

	clock_t start=clock();
	double dt;
	if(fileExt == "osg") 
	{
		CMeshWorkOsgView *osgview = (CMeshWorkOsgView *) mwApp->MeshView();
		osg::ref_ptr<osg::Node> root = osgview->GetSceneModel(0);
		if(!root && m_pAsmMesh) {
			CSGFactory sg;
			root = sg.create(m_pAsmMesh);
		}
		if( root && root.valid() ) {
			bool result = osgDB::writeNodeFile(*root, key);
			dt = (double)(clock()-start)/CLOCKS_PER_SEC;
			str.Format(_T(" in %f seconds."),dt);
			statusStr += str;
			mwApp->SetStatusBarString(statusStr);
		} else {
			mwApp->SetStatusBarString(_T("no mesh scene graph to save."));
		}
	} 
	else if(fileExt == "bdf")
	{
		//pMesh me = m_pAsmMesh->ithInstancedMesh(0);
		//M_toBDF(me,key);
		m_pAsmMesh->toBDF(key);
		dt = (double)(clock()-start)/CLOCKS_PER_SEC;
		str.Format(_T(" in %f seconds."),dt);
		statusStr += str;
		mwApp->SetStatusBarString(statusStr);
	}
	else if(fileExt == "mesh")
	{
		pMWMesh me = m_pAsmMesh->ithInstancedMesh(0);
		M_toMEDIT(me,key);
		dt = (double)(clock()-start)/CLOCKS_PER_SEC;
		str.Format(_T(" in %f seconds."),dt);
		statusStr += str;
		mwApp->SetStatusBarString(statusStr);
	}
	else 
	{
		mwApp->SetStatusBarString(_T("Missing extension. Mesh not saved."));
	}
	delete [] key;

	return;
}
