#pragma once

struct TrackCopies {
    static int copy_count;
    static int move_count;

    int value;
    TrackCopies(int v) : value{v} {};
    TrackCopies() : TrackCopies({}) {};

    TrackCopies(const TrackCopies& rhs) {
        value = rhs.value;
        ++copy_count;
    }

    TrackCopies& operator=(const TrackCopies& rhs) {
        value = rhs.value;
        ++copy_count;
        return *this;
    }

    TrackCopies(TrackCopies&& rhs) noexcept {
        value = rhs.value;
        ++move_count;
    }

    TrackCopies& operator=(TrackCopies&& rhs) noexcept {
        value = rhs.value;
        ++move_count;
        return *this;
    }

    static void reset_counts() {
        copy_count = 0;
        move_count = 0;
    }
};