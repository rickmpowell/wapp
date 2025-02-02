
/*
 *  dlg.cpp
 * 
 *  Dialog boxes
 */

#include "dlg.h"

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