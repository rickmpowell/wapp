
/*
 *  dlg.cpp
 * 
 *  Dialog boxes.
 * 
 *  Dialogs work by taking input parameters in a structure, which is used to
 *  populate the dialog controls, and returns the same structure filled in
 *  with updated values.
 * 
 *  Error checking is performed prior to dismissing a dialog and the dialog
 *  is not dismissed until the errors are clear. This implies it's possible
 *  for the dialog to contain values that are not legal during intermediate
 *  stages, so errors need to detected and handled cleanly, and "illegal"
 *  states are not unusual in normal operation.
 * 
 *  Best practices:
 *      Implement custom controls that represent the types/classes in the 
 *          application.
 *      The custom controls include decoders, parsers, error detection, 
 *          and may hold state in both raw and parsed (i.e., typed) formats
 *      In cases where multiple controls need consistency between them, 
 *          (i.e., error checking is not contained inside a single control
 *          itself), consistency checking should be performed in the dialog 
 *          box, not the controls.
 * 
 *  Controls should implement
 *      SetData - takes application-specific data type
 *      ErrParseData - parses the raw data into app-specific type, reports
 *          errors on failures
 *      DataGet - Returns the applicatoin-specific data
 *      These are not virtual function, since the types are app-specific.
 *          Naming is just a convention.
 *  
 *  Dialogs should implement
 *      constructor with the object we're operating on as a parameter
 *      ErrValidate - validation routine
 *      ExtractData - moves the data from the dialog into the app object
 */

#include "wapp.h"
#include "id.h"

/*
 *  Dialogs
 */

static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm) {

    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            ::EndDialog(hwnd, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }

    return (INT_PTR)FALSE;
}

/*
 *  WND::Dialog
 * 
 *  Lowest level wrapper on the Dialog API. This is equivalent to the WND class.
 */

int WND::Dialog(int rsd)
{
    return (int)::DialogBoxW(app.hinst, MAKEINTRESOURCE(rsd), hwnd, DlgProc);
}

/*
 *  DLG class
 * 
 *  The base class for the dialog.
 */

DLG::DLG(WN& wnOwner) : 
    WN(wnOwner, false), 
    EVD((WN&)*this),
    fEnd(false), 
    val(0)
{
    iwapp.PushEvd(*this);
}

DLG::~DLG()
{
    iwapp.PopEvd();
}

void DLG::ShowCentered(void)
{
    SZ sz = SzRequestLayout(RcInterior());
    SetBounds(RC(iwapp.RcInterior().ptCenter() - sz/2, sz));
    Show(true);
}

CO DLG::CoText(void) const
{
    return coDlgText;
}

CO DLG::CoBack(void) const
{
    return coDlgBack;
}

void DLG::Draw(const RC& rcUpate)
{
    DrawRc(RcInterior().RcInflate(-6), CoBlend(CoText(), CoBack()), 2);
}

bool DLG::FRun(void)
{
    return false;
}

void DLG::End(int val)
{
    Show(false);
    fEnd = true;
    this->val = val;
}

void DLG::Validate(void)
{
}

void DLG::EnterPump(void)
{
    fEnd = false;
    ShowCentered();
    iwapp.SetFocus(this);
}

int DLG::QuitPump(MSG& msg)
{
    Show(false);
    iwapp.SetFocus(pwnParent);
    return val;
}

bool DLG::FQuitPump(MSG& msg) const
{
    return EVD::FQuitPump(msg) || fEnd;
}

bool DLG::FKeyDown(int vk)
{
    unique_ptr<ICMD> pcmd;

    switch (vk) {

    case VK_RETURN:
        pcmd = make_unique<CMDOK>(*this);
        iwapp.FExecuteCmd(*pcmd);
        return true;

    case VK_ESCAPE:
        pcmd = make_unique<CMDCANCEL>(*this);
        iwapp.FExecuteCmd(*pcmd);
        return true;;

    default:
        break;
    }
    return false;
}

/*
 *  Open dialog
 */

DLGFILE::DLGFILE(IWAPP& wapp) :
    DLG(wapp)
{
}

void DLGFILE::BuildFilter(wchar_t* wsFilter, int cchFilter)
{
    wchar_t* pchFilter = wsFilter;
    for (auto it : mpextsLabel) {
        wcsncpy_s(pchFilter, cchFilter - (pchFilter - wsFilter) - 3, WsFromS(it.second).c_str(), _TRUNCATE);   // second has label
        pchFilter += wcslen(pchFilter);
        *pchFilter++ = 0;
        *pchFilter++ = L'*';
        *pchFilter++ = L'.';
        wcsncpy_s(pchFilter, cchFilter - (pchFilter - wsFilter) - 3, WsFromS(it.first).c_str(), _TRUNCATE);
        pchFilter += wcslen(pchFilter);
        *pchFilter++ = 0;
    }
    *pchFilter++ = 0;   // double null-terminate the final filter item
}

DLGFILEOPEN::DLGFILEOPEN(IWAPP& wapp) :
    DLGFILE(wapp)
{
}

OFN DLGFILE::OfnDefault(void)
{
    OFN ofn(1024);
    ofn.ofn.hwndOwner = iwapp.hwnd;
    ofn.ofn.hInstance = iwapp.hinst;
    BuildFilter(ofn.wsFilter.get(), 1024);
    ofn.ofn.lpstrFilter = ofn.wsFilter.get();
    auto it = mpextsLabel.find(extDefault);
    ofn.ofn.nFilterIndex = (int)distance(mpextsLabel.begin(), it) + 1;
    ofn.ofn.lpstrFile = ofn.wsFile.get();
    ofn.ofn.nMaxFile = 1024;
    ofn.ofn.lpstrDefExt = WsFromS(extDefault).c_str();

    wcscpy_s(ofn.wsFile.get(), 1024, WsFromS(path).c_str());

    return move(ofn);
}

bool DLGFILEOPEN::FRun(void)
{
    OFN ofn(move(OfnDefault()));
    ofn.ofn.lpstrTitle = L"Open";
    ofn.ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (!::GetOpenFileNameW(&ofn.ofn))
        return false;

    path = SFromWs(ofn.wsFile.get());
    return true;
}

DLGFILEOPENMULTI::DLGFILEOPENMULTI(IWAPP& wapp) :
    DLGFILEOPEN(wapp)
{
}

bool DLGFILEOPENMULTI::FRun(void)
{
    OFN ofn(move(OfnDefault()));
    ofn.ofn.lpstrTitle = L"Open";
    ofn.ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (!::GetOpenFileNameW(&ofn.ofn))
        return false;

    wchar_t* pwch = ofn.wsFile.get();
    path = SFromWs(pwch);
    for (pwch += wcslen(pwch) + 1; *pwch; pwch += wcslen(pwch) + 1)
        vfile.emplace_back(SFromWs(pwch));
    if (vfile.size() == 0) {
        filesystem::path fpath(path);
        path = fpath.parent_path().string();
        vfile.emplace_back(fpath.filename().string());
    }
    return true;
}

DLGFILESAVE::DLGFILESAVE(IWAPP& wapp) :
    DLGFILE(wapp)
{

}

bool DLGFILESAVE::FRun(void)
{
    OFN ofn(move(OfnDefault()));
    ofn.ofn.lpstrTitle = L"Save";
    ofn.ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;

    if (!::GetSaveFileNameW(&ofn.ofn))
        return false;

    path = SFromWs(ofn.wsFile.get());
    return true;
}

/*
 *  TITLEDLG control
 */

TITLEDLG::TITLEDLG(DLG& dlg, const string& sTitle) :
    STATIC(dlg, sTitle),
    btnclose(*this, new CMDCANCEL(dlg))
{
    SetFont(sFontUI, 40, TF::WEIGHT::Bold);
}

TITLEDLG::TITLEDLG(DLG& dlg, int rssTitle) : 
    STATIC(dlg, rssTitle),
    btnclose(*this, new CMDCANCEL(dlg))
{
    SetFont(sFontUI, 40, TF::WEIGHT::Bold);
}

void TITLEDLG::Layout(void)
{
    RC rc(RcInterior());
    RC rcClose(rc);
    float dxyClose = rc.dyHeight() * 0.5f;
    rcClose.left = rcClose.right - dxyClose;
    rcClose.CenterDy(dxyClose);
    btnclose.SetBounds(rcClose);
}

SZ TITLEDLG::SzRequestLayout(const RC& rcWithin) const
{
    SZ sz(SzFromS(sImage, tf));
    sz.width = max(sz.width, rcWithin.dxWidth());
    return sz;
}

/*
 *  INSTRUCT control. A control for displaying short dialog instructions.
 */

INSTRUCT::INSTRUCT(DLG& dlg, const string& sText) :
    STATICL(dlg, sText, rssInstructionBulb)
{
    SetFont(sFontSymbol, 16);
}

INSTRUCT::INSTRUCT(DLG& dlg, int rssText) :
    STATICL(dlg, rssText, rssInstructionBulb)
{
    SetFont(sFontSymbol, 16);
}

void INSTRUCT::DrawLabel(const RC& rcLabel)
{
    DrawSCenter(sLabel, tf, rcLabel, coYellow);
}