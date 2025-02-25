#include <windows.h>
#include <memory>

class base
{
public:
    virtual void execute(void) = 0;
};

class derived : public base
{
public:
    virtual void execute(void) override {
    }
};

void dispatch(base& item)
{
    item.execute();
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    derived cmd;
    dispatch(cmd);
    return 1;
}