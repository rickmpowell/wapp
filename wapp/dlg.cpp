
/*
 *  dlg.cpp
 * 
 *  Dialog boxes
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
    return coWhite;
}

CO DLG::CoBack(void) const
{
    return coDlgBackLight;
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