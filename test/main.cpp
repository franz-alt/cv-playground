#include <gtest/gtest.h>

#ifdef USE_TCMALLOC
#include <gperftools/malloc_extension.h>
#endif

int main(int argc, char * argv[])
{
#ifdef USE_TCMALLOC
    MallocExtension::Initialize();
#endif

    ::testing::InitGoogleTest(&argc, argv);

    auto result = RUN_ALL_TESTS();

#ifdef USE_TCMALLOC
    char buffer[20000];

    MallocExtension::instance()->GetStats(&buffer[0], sizeof(buffer));

    std::cout << "TCMalloc:" << std::endl;
    std::cout << buffer << std::endl;
#endif

    return result;
}
