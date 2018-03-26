//$c10 XRL 11/03/13 add selection manager
//$c9 XRL 10/09/13 support multiple assembly meshes.
//$c8 XRL 03/29/13 add "m_picked" in PS view.
//$c7 XRL 07/13/12 add access to osgview from meshdoc.
//$c6 XRL 07/03/12 start to Manage MeshControl here.
//$c5 XRL 12/23/11 change m_parts into m_topols and added m_topols 
//$c4 XRL 11/23/11 add pointer to m_mesh
//$c3 XRL 03/22/11 update to parasolid v23 and integrated BLpara.
//$c2 XRL 02/03/11 added topol/body/part picking and show its tag in GUI.
//$c1 XRL 02/01/11 created.
//=========================================================================
//
// MeshWorkDoc.h : declear CMeshWorkDoc class
//
//=========================================================================


#ifndef MESHWORKDOC_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_
#define MESHWORKDOC_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_

#include <vector>
#include <list>
#include "meshing/AssemblyMeshMgr.h"
#include "osg/SelectionMgr.h"

typedef struct 
{
	int occurence;	// index to flattened body and transform array
	PK_ENTITY_t tag;	
} InstancedTag;

// Forward definition 
class CMeshWorkView;

class CMeshWorkDoc : public CDocument
{
protected: // create from serialization only
	CMeshWorkDoc();
	DECLARE_DYNCREATE(CMeshWorkDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshWorkDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	
	/////////////////////  view data  ////////////////////////
	static CMeshWorkView* m_view;
	PK_VECTOR_t m_viewCentre;
	ViewStyle m_viewStyle;
	double m_bSphereRad;
	double m_scaleFactor;
	static PK_BODY_type_t m_currentBodyType;
	std::list<InstancedTag> m_picked;
	
	// GO functions
	static void CoutputSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail);
	static void CcloseSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail);
	static void CopenSegment(const int* segtyp, const int* ntags, const int* tags, 
							 const int* ngeoms, const double* geoms, 
							 const int* nlntp, const int* lntp, int* ifail);

	CSelectionMgr *getSelectionMgr() { return m_pSelectionMgr; }

	void setAssemblyMesh(pAssemblyMesh);
	pAssemblyMeshMgr getAssemblyMeshMgr() { return m_pAsmMeshMgr; }
	pMeshControl getLocalMeshControl() { return m_pMeshCtrl;}

	////////////////// parasoild model data  //////////////////////////				
	// the loaded parasolid model in view			01/31/2011  XRL
	// flatten all assemblies						12/22/2011	XRL
	// set in OnOpenDocument
	int m_ntopol;				// number of instanced bodies (instead of referenced)
	PK_TOPOL_t* m_topols;		// the body array
	PK_TRANSF_t* m_transfs;		// the flatten transform array. NULL if no assembly
	int m_ngeoms;				// number of geometries, usually zero
	PK_GEOM_t* m_geoms;			// the geometry array

	void AddPSModel(int);
	void InsertPSphere4P();
	static int m_psModelIndex;

private:
	////////////////// Selection manager //////////////////////////
	CSelectionMgr *m_pSelectionMgr;

	////////////////////// mesh data  /////////////////////////////
	pAssemblyMeshMgr m_pAsmMeshMgr;
	pMeshControl m_pMeshCtrl;

	PK_SESSION_frustrum_t m_frustrum;
	PK_SESSION_frustrum_t m_oldfrustrum;

	virtual ~CMeshWorkDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// GO helper functions
	static void AddCircle( const double* centre, const double* normal, double radius,
							const double* start, const double* end );
	static void AddEllipse(const double * centre, const double * majorAxis, 
						   const double * minorAxis, double majorRadius, 
						   double minorRadius, const double * start, const double * end);
	static int ParseLineData( int segtyp, int ntags, const int* tags, 
							 int ngeoms, const double* geoms, 
							 int nlntp, const int* lntp );
	static void AddTriangle( const double* positions, const double* normals );

// Generated message map functions
protected:
	//{{AFX_MSG(CMeshWorkDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    // have to save since OpenDocument can change it		// 2/2/2011 XRL
    TCHAR curworkingdir[MAX_PATH];

	//
	void clear();
	void LoadPSModel(char[255]);
	void UpdatePSView();
	int LoadBDF(char[255], bool);
	int LoadPointCloud(char[255], bool);
	int LoadIBL(char[255], bool);
	int receive_parts(char[255], int *, PK_PART_t** );
	void assembly_ask_bodies_transfs(PK_ASSEMBLY_t, PK_TRANSF_t, std::vector<PK_BODY_t> &, std::vector<PK_TRANSF_t> &);

public:
	afx_msg void OnFileSaveas();
	afx_msg void OnFileInsert();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXAMPLEAPPDOC_H__CD27374F_D581_11D2_8BF9_0000F8071DC8__INCLUDED_)
