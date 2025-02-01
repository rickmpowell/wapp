#include <windows.h>

class command
{
public:
    virtual ~command() = default;
    virtual void execute() = 0;
    virtual command* clone(void) const = 0;
};

template <typename d>
class helper : public command
{
public:
    int app_;
    helper(int app) : app_(app) {}
    virtual command* clone(void) const override {
        return new d(static_cast<const d&>(*this));
    }
};

class fifty : public helper<fifty>
{
public:
    fifty(int app) : helper(app) {}

    void virtual execute(void) override {
        app_ *= 2;
    }


};

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
    fifty cmd(0);
    cmd.execute();
    return 1;
}
