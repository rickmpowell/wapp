#include <windows.h>
#include <memory>
#include <vector>


class MV
{
public:
    MV(int sqFrom, int sqTo) :
        sqFrom(sqFrom), sqTo(sqTo)
    {
    }

private:
    int sqFrom, sqTo;
};

class MVU : public MV
{
public:
    MVU(const MV& mv, int cp) :
        MV(mv),
        cpTake(cp)
    {
    }

private:
    int cpTake;
};


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    return 1;
}

std::vector<MVU> vmvu;

void Test(MV mv)
{
    vmvu.emplace_back(mv, 7);

}
