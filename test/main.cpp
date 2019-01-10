#include <locale>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_scan.h"
#include "test_compiler.h"

using namespace std;

int main(int argc, char *argv[])
{
    locale::global(std::locale(""));

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

