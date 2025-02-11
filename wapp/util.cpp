#include "wapp.h"


wstring WsFromS(const string& s)
{
    return wstring(s.begin(), s.end());
}