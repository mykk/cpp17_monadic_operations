#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

int TrackCopies::copy_count = 0;
int TrackCopies::move_count = 0;


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
