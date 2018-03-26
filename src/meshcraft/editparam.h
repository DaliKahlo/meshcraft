//$c1 XRL 01/05/12 Created.
//
//========================================================================//
//
//     MeshWorks Edit control
//
//========================================================================//
#if !defined(AFX_EDITPARAM_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_)
#define AFX_EDITPARAM_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditParam.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditParam window

class CEditParam : public CEdit
{
// Construction
public:
	CEditParam();

// Attributes
public:

// Operations
public:
	void	SetRclickMenu(BOOL bEnable) { m_bRclickMenu = bEnable; }
	void	ResetParamExpression(BOOL bEnable);
	void	GetUnitTypeAndUnit(UINT& nUnitType, UINT& nUnit);
	void	SetUnitTypeAndUnit(UINT nUnitType, UINT nUnit);
	BOOL	IsParamExpression() { return m_bUseParamExpression; }
	CString GetParamExpression() { return m_sParamExpression; }
	void	SetParamExpression(CString sParamExpression = _T("")); 
	BOOL	LinkParameter();
	void	UnlinkParameter();
	CString GetParameterValue() { return m_strValue; }
	
	virtual void SetParametricValue();
	void		DeleteBrushObjects();
	
	HBRUSH	CtlColor(CDC* pDC, UINT nCtlColor);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditParam)
	public:
	virtual BOOL PreTranslateMessage( MSG* pMsg );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditParam();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditParam)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditDefineParamExpression();
	afx_msg void OnUseParamExpression();
	afx_msg void OnUpdateUseParamExpression(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
//	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	virtual BOOL EditDefineParamExpression();

protected:
	UINT	 m_nUnitType;
	UINT	 m_nUnit;
	BOOL     m_bUseParamExpression;
	CString  m_sParamExpression;
	CBrush	 m_BackBrush;
	CBrush	 m_BackBrush1;
	CBrush	 m_InvalidBackBrush;
	BOOL     m_bInvalidExpression;
	BOOL     m_bRclickMenu;
	CString	 m_strValue;
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CEditParamDouble window

class CEditParamDouble : public CEditParam
{
// Construction
public:
	CEditParamDouble();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditParamDouble)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditParamDouble();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditParamDouble)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	BOOL EditDefineParamExpression();
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CEditParamInt window

class CEditParamInt : public CEditParam
{
// Construction
public:
	CEditParamInt();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditParamInt)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditParamInt();

	// Generated message map functions
protected:
	BOOL EditDefineParamExpression();

	//{{AFX_MSG(CEditParamInt)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPARAM_H__E1C38130_3A61_11D5_9107_00105AA7521E__INCLUDED_)
