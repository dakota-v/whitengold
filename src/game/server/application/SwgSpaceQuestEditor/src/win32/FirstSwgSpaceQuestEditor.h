// ======================================================================
//
// FirstSwgSpaceQuestEditor.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_FirstSwgSpaceQuestEditor_H
#define INCLUDED_FirstSwgSpaceQuestEditor_H

// ======================================================================

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

// ======================================================================

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif 

#include <afxcview.h>

#include <afxtempl.h>
#include <afxpriv.h>
#include <afxole.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef std::vector<CString> StringList;
typedef std::set<CString> StringSet;

// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"

// ======================================================================

//{{AFX_INSERT_LOCATION}}

// ======================================================================

#endif 
