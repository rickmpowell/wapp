
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

#ifndef CONSOLE

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
}

DLG::~DLG()
{
}

void DLG::ShowCentered(void)
{
    SZ sz = SzIntrinsic(RcInterior());
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
    iwapp.PushEvd(*this);
    fEnd = false;
    ShowCentered();
    iwapp.SetFocus(this);
}

int DLG::QuitPump(MSG& msg)
{
    Show(false);
    iwapp.PopEvd();
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
    fVisible = false;
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
    ofn.ofn.lpstrInitialDir = ofn.wsDirectory.get();

    if (!file.empty()) {
        wcscpy_s(ofn.wsFile.get(), 1024, WsFromS(file.filename().string()).c_str());
        wcscpy_s(ofn.wsDirectory.get(), 1024, WsFromS(file.parent_path().string()).c_str());
    }

    return move(ofn);
}

bool DLGFILEOPEN::FRun(void)
{
    OFN ofn(move(OfnDefault()));
    ofn.ofn.lpstrTitle = L"Open";
    ofn.ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (!::GetOpenFileNameW(&ofn.ofn))
        return false;

    file = SFromWs(ofn.wsFile.get());
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
    folder = SFromWs(pwch);
    for (pwch += wcslen(pwch) + 1; *pwch; pwch += wcslen(pwch) + 1)
        vfile.emplace_back(SFromWs(pwch));
    if (vfile.size() == 0) {
        vfile.emplace_back(folder.filename().string());
        folder = folder.parent_path().string();
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

    file = SFromWs(ofn.wsFile.get());
    return true;
}

/*
 *  DLGFOLDER dialog
 */

DLGFOLDER::DLGFOLDER(IWAPP& wapp) :
    DLG(wapp)
{
}

bool DLGFOLDER::FRun(void)
{
    com_ptr<IFileDialog> pdlg;
    ThrowError(CoCreateInstance(CLSID_FileOpenDialog, 
                                NULL, 
                                CLSCTX_INPROC_SERVER,
                                __uuidof(pdlg),
                                &pdlg));
    DWORD opt;
    ThrowError(pdlg->GetOptions(&opt));
    ThrowError(pdlg->SetOptions(opt| FOS_PICKFOLDERS));
    HRESULT hr = pdlg->Show(NULL);
    if (!SUCCEEDED(hr))
        return false;
    com_ptr<IShellItem> psi;
    pdlg->GetResult(&psi);
    PWSTR wsPath = nullptr;
    psi->GetDisplayName(SIGDN_FILESYSPATH, &wsPath);
    folder = SFromWs(wstring_view(wsPath));
    CoTaskMemFree(wsPath);
    return true;
}

/*
 *  DLGPRINT dialog
 */

DLGPRINT::DLGPRINT(IWAPP& wapp) :
    DLG(wapp)
{
}

bool DLGPRINT::FRun(void)
{
    PRINTDLGEXW pd = { sizeof(pd) };
    pd.hwndOwner = iwapp.hwnd;
    pd.Flags = PD_RETURNDC | PD_NOPAGENUMS;
    pd.nCopies = 1;
    pd.nStartPage = START_PAGE_GENERAL;
    pd.hDevNames = NULL; 
    
    global_ptr<DEVMODEW> pdevmode(1, GMEM_MOVEABLE | GMEM_ZEROINIT);
    pd.hDevMode = pdevmode.handle();
    pdevmode->dmSize = sizeof(DEVMODEW);
    pdevmode->dmFields = DM_ORIENTATION | DM_PAPERSIZE;
    pdevmode->dmOrientation = DMORIENT_LANDSCAPE;
    pdevmode->dmPaperSize = DMPAPER_LETTER;
    pdevmode.unlock();

    HRESULT hr = ::PrintDlgEx(&pd);
    if (!SUCCEEDED(hr) || pd.dwResultAction != PD_RESULT_PRINT || pd.hDC == NULL)
        return false;

    hdc = pd.hDC;
    return true;
}

/*
 *  DLGABOUT
 */

DLGABOUT::DLGABOUT(IWAPP& wapp) :
    DLG(wapp),
    title(*this, rssAboutTitle),
    instruct(*this, rssAboutInstruct),
    copyright(*this, rssAboutCopyright),
    icon(*this, rsiApp),
    btnok(*this)
{
    instruct.SetFontHeight(16);
    copyright.SetFontHeight(16);
}

void DLGABOUT::Layout(void)
{
    LENDLG len(*this);
    len.Position(title);
    len.AdjustMarginDy(dxyDlgGutter / 2);
    len.StartFlow();
    len.PositionLeft(icon);
    LEN len2(len.RcFlow(), PAD(0), PAD(16));
        len2.Position(instruct);
        len2.Position(copyright);
    len.EndFlow();
    len.PositionOK(btnok);
}

SZ DLGABOUT::SzIntrinsic(const RC& rcWithin)
{
    return SZ(700, 360);
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

SZ TITLEDLG::SzIntrinsic(const RC& rcWithin)
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
    SetFont(sFontUI, 16);
}

INSTRUCT::INSTRUCT(DLG& dlg, int rssText) :
    STATICL(dlg, rssText, rssInstructionBulb)
{
    SetFont(sFontUI, 16);
}

void INSTRUCT::DrawLabel(const RC& rcLabel)
{
    DrawSCenter(sLabel, tf, rcLabel, coYellow);
}

#endif