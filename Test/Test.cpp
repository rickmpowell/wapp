#define WINVER 0x0500
#include <windows.h>
#include <wrl.h>
#include <string>

class TESTC
{
public:
    TESTC(int sqFrom, int sqTo) :
        sqFrom(sqFrom), sqTo(sqTo)
    {
    }

    std::string to_string(void) const
    {
        return std::string(1, sqFrom + 'a');
    }

private:
    int sqFrom, sqTo;
};


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    PRINTDLGEX pd = { sizeof pd };
    ::PrintDlgEx(&pd);

    return 1;
}
