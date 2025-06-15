
#if 1
#include <string>

#include "ectbox/ectbox.h"
#include "include/tbox.h"

Tbox *tbox = nullptr;

int hello()
{
    std::cout << "hello" << std::endl;
    return 0;
}

Tbox *create_Tbox(std::string &tboxName)
{
    EcTbox *ectbox;
    ectbox = new EcTbox();
    return ectbox;
}

int loadTbox2(std::string &tboxName)
{
    // tbox = loadTbox(tboxName);
    // tbox = new EcTbox();
    tbox = create_Tbox(tboxName);
    return 0;
}

int findNic()
{
    tbox->findNic();
    return 0;
}

#endif
