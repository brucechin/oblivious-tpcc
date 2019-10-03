#include "randomgenerator.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>



namespace tpcc {

NURandC NURandC::makeRandom(RandomGenerator* generator) {
    NURandC c;
    c.c_last_ = generator->number(0, 255);
    c.c_id_ = generator->number(0, 1023);
    c.ol_i_id_ = generator->number(0, 8191);
    return c;
}

// Returns true if the C-Run value is valid. See TPC-C 2.1.6.1 (page 20).
static bool validCRun(int cRun, int cLoad) {
    int cDelta = abs(cRun - cLoad);
    return 65 <= cDelta && cDelta <= 119 && cDelta != 96 && cDelta != 112;
}

NURandC NURandC::makeRandomForRun(RandomGenerator* generator, const NURandC& c_load) {
    NURandC c = makeRandom(generator);

    while (!validCRun(c.c_last_, c_load.c_last_)) {
        c.c_last_ = generator->number(0, 255);
    }

    return c;
}

int RandomGenerator::numberExcluding(int lower, int upper, int excluding) {


    // Generate 1 less number than the range
    int num = number(lower, upper-1);

    // Adjust the numbers to remove excluding
    if (num >= excluding) {
        num += 1;
    }
    return num;
}


float RandomGenerator::fixedPoint(int digits, float lower, float upper) {
    int multiplier = 1;
    for (int i = 0; i < digits; ++i) {
        multiplier *= 10;
    }

    int int_lower = static_cast<int>(lower * static_cast<double>(multiplier) + 0.5);
    int int_upper = static_cast<int>(upper * static_cast<double>(multiplier) + 0.5);
    return (float) number(int_lower, int_upper) / (float) multiplier;
}

int RandomGenerator::NURand(int A, int x, int y) {
    int C = 0;
    switch(A) {
        case 255:
            C = c_values_.c_last_;
            break;
        case 1023:
            C = c_values_.c_id_;
            break;
        case 8191:
            C = c_values_.ol_i_id_;
            break;
        default:
            fprintf(stderr, "Error: NURand: A = %d not supported\n", A);
            exit(1);
    }
    return (((number(0, A) | number(x, y)) + C) % (y - x + 1)) + x;
}

int* RandomGenerator::makePermutation(int lower, int upper) {
    // initialize with consecutive values
    int* array = new int[upper - lower + 1];
    for (int i = 0; i <= upper - lower; ++i) {
        array[i] = lower + i;
    }

    for (int i = 0; i < upper - lower; ++i) {
        // choose a value to go into this position, including this position
        int index = number(i, upper - lower);
        int temp = array[i];
        array[i] = array[index];
        array[index] = temp;
    }

    return array;
}



RealRandomGenerator::RealRandomGenerator() {
#ifdef HAVE_RANDOM_R
    // Set the random state to zeros. glibc will attempt to access the old state if not NULL.
    memset(&state, 0, sizeof(state));
    int result = initstate_r(static_cast<unsigned int>(time(NULL)), state_array,
            sizeof(state_array), &state);
#else
    seed(time(NULL));
#endif
}

int RealRandomGenerator::number(int lower, int upper) {
    int rand_int;
#ifdef HAVE_RANDOM_R
    int error = random_r(&state, &rand_int);
#else
    rand_int = nrand48(state);
#endif

    // Select a number in [0, range_size-1]
    int range_size = upper - lower + 1;
    rand_int %= range_size;

    // Shift the range to [lower, upper]
    rand_int += lower;
    return rand_int;
}

void RealRandomGenerator::seed(unsigned int seed) {
#ifdef HAVE_RANDOM_R
    int error = srandom_r(seed, &state);
#else
    memcpy(state, &seed, std::min(sizeof(seed), sizeof(state)));
#endif
}

}  // namespace tpcc
