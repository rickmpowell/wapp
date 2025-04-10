
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
 *
 */

DLG::DLG(WN& wnOwner) : 
    WN(wnOwner, false), fEnd(false), val(0)
{
}

void DLG::ShowCentered(void)
{
    SZ sz = SzRequestLayout();
    SetBounds(RC(iwapp.RcInterior().ptCenter() - sz/2.0f, sz));
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
    DrawRc(RcInterior().RcInflate(-6.0f), CoAverage(CoText(), CoBack()), 2.0f);
}

void DLG::EndDlg(int val)
{
    Show(false);
    fEnd = true;
    this->val = val;
}

void DLG::Validate(void)
{
}

int DLG::DlgMsgPump(void)
{
    fEnd = false;
    ShowCentered();

    MSG msg;
    while (::GetMessage(&msg, nullptr, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (fEnd)
            break;
    }

    Show(false);
    
    return val;
}