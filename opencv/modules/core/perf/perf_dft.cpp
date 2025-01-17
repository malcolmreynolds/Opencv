#include "perf_precomp.hpp"

using namespace std;
using namespace cv;
using namespace perf;

#define MAT_TYPES_DFT  CV_32FC1, CV_64FC1
#define MAT_SIZES_DFT  sz1080p, sz2K
#define TEST_MATS_DFT testing::Combine( testing::Values(MAT_SIZES_DFT), testing::Values(MAT_TYPES_DFT) )

PERF_TEST_P(Size_MatType, dft, TEST_MATS_DFT)
{
    Size sz = std::tr1::get<0>(GetParam());
    int type = std::tr1::get<1>(GetParam());

    Mat src(sz, type);
    Mat dst(sz, type);
 
    declare.in(src, WARMUP_RNG);
    declare.time(60);

    TEST_CYCLE(100) 
    {
        dft(src, dst);
    }

    SANITY_CHECK(dst);
} 
