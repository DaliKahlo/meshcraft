//$c2 PBH 10/03/07 SPR 397588 SetResourceHandle
//$c1 PEL 09/17/07 Created.
//========================================================================//
//              Copyright 2007 (Unpublished Material)                     //
//                  SolidWorks Corp.                                      //
//========================================================================//
//
//
//     Application: Cosmosworks
//
//
//========================================================================//
// EditParam.cpp : implementation file
//

#include "stdafx.h"
#include <afxtempl.h>
#include <atlbase.h>
//#include "paramutility.h"
#include "EditParam.h"
//#include "ParamExpressionDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern MeshWorks* meshApp;

/////////////////////////////////////////////////////////////////////////////
// CEditParam

CEditParam::CEditParam()
{
	m_bUseParamExpression = FALSE;
	m_BackBrush1.CreateSolidBrush(::GetSysColor(COLOR_WINDOW)); 
	m_nUnitType = kParamNoUnits;
	m_nUnit = 0;
	m_sParamExpression = _T("");
	m_BackBrush.CreateSolidBrush( RGB(0, 0, 255) );
	m_InvalidBackBrush.CreateSolidBrush( RGB(255, 0, 0) );
	m_bInvalidExpression = FALSE;
	m_bRclickMenu = TRUE;
	m_strValue = _T("");
}

CEditParam::~CEditParam()
{
	DeleteBrushObjects();
}


BEGIN_MESSAGE_MAP(CEditParam, CEdit)
	//{{AFX_MSG_MAP(CEditParam)
//	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_DGXPARAMETRIC_EDIT_DEFINE, OnEditDefineParamExpression)
	ON_COMMAND(ID_DGXPARAMETRIC_ENABLE_EDIT, OnUseParamExpression)
	ON_WM_CHAR()
//	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditParam message handlers

void CEditParam::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (GetSafeHwnd() == NULL) return;
	// TODO: Add your message handler code here and/or call default
	CEdit::OnLButtonDblClk(nFlags, point);

	EditDefineParamExpression();
}

void CEditParam::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (GetSafeHwnd() == NULL) return;
	// TODO: Add your message handler code here and/or call default
	SetModify(FALSE);
	SetFocus();
	Invalidate();

	if (m_bRclickMenu)
	{
		CMenu	menu;
		POINT	pt;

		GetCursorPos( &pt );
		
		CwLoadMenu(menu, IDR_PARAMETRIC_MENU);
		//menu.LoadMenu(IDR_PARAMETRIC_MENU);
		menu.EnableMenuItem(ID_DGXPARAMETRIC_ENABLE_EDIT, (m_bUseParamExpression ? MF_ENABLED : MF_GRAYED) );
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, pt.x,pt.y,this);
	}	
//	CEdit::OnRButtonDown(nFlags, point);
}

void CEditParam::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (GetSafeHwnd() == NULL) return;
	SetFocus();
	Invalidate();
	if (m_bUseParamExpression ) return;

	CEdit::OnLButtonDown(nFlags, point);
}

void CEditParam::OnEditDefineParamExpression() 
{
	EditDefineParamExpression();
}

BOOL CEditParam::EditDefineParamExpression()
{
	return FALSE;
}

void CEditParam::OnUseParamExpression() 
{
	// TODO: Add your command handler code here
	m_bUseParamExpression = !m_bUseParamExpression;	
//	SetWindowText(_T(""));
	if (GetSafeHwnd() != NULL)
	{
		SetFocus();
		Invalidate();
	}
	if (!m_bUseParamExpression)
	{
		m_sParamExpression = _T("");
		if (GetSafeHwnd() != NULL)
			::SendMessage( GetParent()->GetSafeHwnd(), WM_USERLINKPARAMETER, 0, 0 );
	}
}

void CEditParam::OnUpdateUseParamExpression(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bUseParamExpression);
}

void CEditParam::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (GetSafeHwnd() == NULL) return;
	if (m_bUseParamExpression) return;
		
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

HBRUSH CEditParam::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if (GetSafeHwnd() == NULL) return NULL;
	COLORREF clrText = RGB(255, 255, 255);
	if (m_bUseParamExpression)
	{
//		clrText = RGB(127, 0, 127);
		clrText = RGB(255, 255, 255);
		pDC->SetTextColor(clrText);
		pDC->SetBkMode(TRANSPARENT);
	}

	return (m_bUseParamExpression && !(GetStyle() & WS_DISABLED) ? (HBRUSH)m_BackBrush : (HBRUSH)NULL);
}

void CEditParam::SetParamExpression(CString sParamExpression) 
{ 
	m_sParamExpression = sParamExpression; 
	if ( sParamExpression.GetLength() > 0 )
	{
		m_bUseParamExpression = TRUE;

		HRESULT hr;
		CComPtr<IParametricManager> spParametricModel;
		spParametricModel = CosmosApp->GetParametricModel();
		if (!spParametricModel) return;
		CComPtr<IParametricObject> spInfo;
		BSTR bstrParam = m_sParamExpression.AllocSysString();
		hr = spParametricModel->Find(bstrParam, &spInfo);
		SysFreeString(bstrParam);
		if (SUCCEEDED(hr)) 
		{
			BOOL bIsExpression = FALSE;
			spInfo->get_isExpression(&bIsExpression);
			BOOL bIsValidExpression = TRUE;
			spInfo->get_IsValidExpression(&bIsValidExpression);
			m_bInvalidExpression = (bIsExpression && !bIsValidExpression);
		}
	}
}

void CEditParam::SetUnitTypeAndUnit(UINT nUnitType, UINT nUnit)
{
	m_nUnitType = nUnitType;
	m_nUnit		= nUnit;
}

void CEditParam::GetUnitTypeAndUnit(UINT& nUnitType, UINT& nUnit)
{
	nUnitType = m_nUnitType;
	nUnit	  = m_nUnit;
}

void CEditParam::ResetParamExpression(BOOL bEnable)
{
	m_bUseParamExpression = bEnable;
	if ( !bEnable )
	{
		m_sParamExpression = _T("");
	}
}

void CEditParam::SetParametricValue()
{
	if (!m_bUseParamExpression) return;

	HRESULT hr;
	CComPtr<IParametricManager> spParametricModel;
	spParametricModel = CosmosApp->GetParametricModel();
	if (!spParametricModel) return;
	CComPtr<IParametricObject> spInfo;
	BSTR bstrParam = m_sParamExpression.AllocSysString();
	hr = spParametricModel->Find(bstrParam, &spInfo);
	SysFreeString(bstrParam);
	if (FAILED(hr)) 
	{
		ResetParamExpression(FALSE);
		return;
	}

	double dValue;
	short nUnitType, nUnit;
	spInfo->get_UnitType(&nUnitType);
	UINT nType = nUnitType;
	if (nType != m_nUnitType)
		ResetParamExpression(FALSE);
	else
	{
		spInfo->get_Unit(&nUnit);
		spInfo->get_DoubleValue(&dValue);
		if (m_nUnitType == kParamTemperatureUnits)
		{
			dValue = TemperatureFromUnits(dValue, nUnit, m_nUnit);
		}
		else
		{
			dValue = dValue * GetParametricUnitFactor(m_nUnitType, nUnit, m_nUnit);
		}

		CString strValue;
		strValue.Format(_T("%g"), dValue);
		m_strValue = strValue;
		if (GetSafeHwnd() != NULL)
		{
			//CString strValue;
			//strValue.Format(_T("%g"), dValue);
			SetWindowText(strValue);
		}
		
		BOOL bIsExpression = FALSE;
		spInfo->get_isExpression(&bIsExpression);
		BOOL bIsValidExpression = TRUE;
		spInfo->get_IsValidExpression(&bIsValidExpression);
		m_bInvalidExpression = (bIsExpression && !bIsValidExpression);
	}
}

BOOL CEditParam::OnEraseBkgnd(CDC* pDC)
{
	if (GetSafeHwnd() == NULL) return FALSE;
	if (m_bUseParamExpression)
	{
		CRect rcFill;
		GetClientRect(&rcFill);
		pDC->FillRect(&rcFill, (m_bInvalidExpression? &m_InvalidBackBrush : &m_BackBrush) );
	}
	else
	{
		CRect rcFill;
		GetClientRect(&rcFill);
		pDC->FillRect(&rcFill, &m_BackBrush1);
	}

	return TRUE;
}

void CEditParam::OnPaint()
{
	if (GetSafeHwnd() == NULL) return;
	if (m_bUseParamExpression && IsWindowEnabled())
	{
		CPaintDC dc(this);
		CRect rc;
		GetClientRect(&rc);
//		dc.SetTextColor(RGB(127, 0, 127));
		dc.SetTextColor(RGB(255, 255, 255));
		dc.FillRect(&rc, (m_bInvalidExpression? &m_InvalidBackBrush : &m_BackBrush));
		dc.SetBkMode(TRANSPARENT);
		CString strText;
		GetWindowText(strText);

		CFont *pOldFont = (CFont *) dc.SelectObject( GetFont( ) );
		dc.DrawText(strText, -1, &rc, DT_LEFT);
		dc.SelectObject(pOldFont);
		return;
	}

	CEdit::OnPaint();
}

BOOL CEditParam::PreTranslateMessage(MSG* pMsg) 
{
	if (GetSafeHwnd() == NULL) return FALSE;
	BOOL bHadSysTimerMsg = (pMsg->message == 0x118); // WM_SYSTIMER (undocumented)

    if ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) ||
		(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)) 
    {
		if (pMsg->message == WM_KEYUP && !bHadSysTimerMsg &&
			 (pMsg->wParam == VK_TAB))
		{
			SetSel (0, -1);
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

BOOL CEditParam::LinkParameter()
{
	return EditDefineParamExpression();
}

void CEditParam::UnlinkParameter()
{
	m_bUseParamExpression = FALSE;	
	m_sParamExpression = _T("");
}

/////////////////////////////////////////////////////////////////////////////
// CEditParamDouble

CEditParamDouble::CEditParamDouble()
{
}

CEditParamDouble::~CEditParamDouble()
{
}


BEGIN_MESSAGE_MAP(CEditParamDouble, CEditParam)
	//{{AFX_MSG_MAP(CEditParamDouble)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditParamDouble message handlers

BOOL CEditParamDouble::EditDefineParamExpression()
{
	BOOL bLinkParameter = FALSE;
	HINSTANCE hCurrentInstance = CwSetResourceHandle();
	CParamExpressionDlg dlg(m_sParamExpression, m_nUnitType, this);
	if (dlg.DoModal() == IDOK)
	{
		m_bUseParamExpression = FALSE;
		double dValue;
		if (m_nUnitType == kParamTemperatureUnits)
		{
			dValue = TemperatureFromUnits(dlg.m_dValue, dlg.m_nUnit, m_nUnit);
		}
		else
		{
			dValue = dlg.m_dValue * GetParametricUnitFactor(m_nUnitType, dlg.m_nUnit, m_nUnit);
		}

		CString strValue;
		strValue.Format(_T("%g"), dValue);
		m_strValue = strValue;
		if (GetSafeHwnd() != NULL)
		{
			SetWindowText(strValue);
		}
		m_bUseParamExpression = TRUE;
		m_sParamExpression = dlg.m_strExpression;
		m_bInvalidExpression = FALSE;
		if (GetSafeHwnd() != NULL)
			::SendMessage( GetParent()->GetSafeHwnd(), WM_USERLINKPARAMETER, 1, 0 );
		bLinkParameter = TRUE;
	}
	AfxSetResourceHandle(hCurrentInstance);

	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
	return bLinkParameter;
}

/////////////////////////////////////////////////////////////////////////////
// CEditParamInt

CEditParamInt::CEditParamInt()
{
}

CEditParamInt::~CEditParamInt()
{
}


BEGIN_MESSAGE_MAP(CEditParamInt, CEditParam)
	//{{AFX_MSG_MAP(CEditParamInt)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditParamInt message handlers

BOOL CEditParamInt::EditDefineParamExpression()
{
	BOOL bLinkParameter = FALSE;
	HINSTANCE hCurrentInstance = CwSetResourceHandle();
	CParamExpressionDlg dlg(m_sParamExpression, m_nUnitType, this);
	if (dlg.DoModal() == IDOK)
	{
		m_bUseParamExpression = FALSE;
		int nValue;
		if (m_nUnitType == kParamTemperatureUnits)
		{
			nValue = (int)TemperatureFromUnits(dlg.m_dValue, dlg.m_nUnit, m_nUnit);
		}
		else
		{
			nValue = (int)(dlg.m_dValue * GetParametricUnitFactor(m_nUnitType, dlg.m_nUnit, m_nUnit));
		}

		CString strValue;
		strValue.Format(_T("%d"), nValue);
		m_strValue = strValue;
		if (GetSafeHwnd() != NULL)
		{
			SetWindowText(strValue);
		}
		m_bUseParamExpression = TRUE;
		m_sParamExpression = dlg.m_strExpression;
		m_bInvalidExpression = FALSE;
		if (GetSafeHwnd() != NULL)
			::SendMessage( GetParent()->GetSafeHwnd(), WM_USERLINKPARAMETER, 1, 0 );
		bLinkParameter = TRUE;
	}
	AfxSetResourceHandle(hCurrentInstance);
	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
	return bLinkParameter;
}

void CEditParam::DeleteBrushObjects()
{
	m_BackBrush1.DeleteObject();					
	m_BackBrush.DeleteObject();					
	m_InvalidBackBrush.DeleteObject();
}



