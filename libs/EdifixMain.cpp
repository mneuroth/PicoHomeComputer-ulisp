
#include "EdifixEditor.h"

extern "C" int __cdecl main(int argc, char *argv[])
{
    EdifixEditor aEditor("test main", 0);
    
    aEditor.Run();
    
    return 0;
}