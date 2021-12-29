#include "configcenter/test_configcenter.h"
#include "dbg/test_dbg.h"
#include "reflection/test_reflection.h"
#include "skip/test_skip.h"
#include "test/mmpv/test_mmpv.h"

int main(int argc, char* argv[])
{
    //test_skip();
    //test_dbg();
    //test_reflection();
    test_configcenter();
    //test_mmpv();
    return 0;
}
