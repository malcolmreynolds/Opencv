#include "perf_precomp.hpp"

using namespace std;
using namespace cv;
using namespace perf;
using std::tr1::make_tuple;
using std::tr1::get;

typedef std::tr1::tuple<String, int, bool, std::tr1::tuple<double, double> > Img_Aperture_L2_thresholds_t;
typedef perf::TestBaseWithParam<Img_Aperture_L2_thresholds_t> Img_Aperture_L2_thresholds;

PERF_TEST_P( Img_Aperture_L2_thresholds, canny,
             testing::Combine(
                 testing::Values( "cv/shared/lena.jpg", "stitching/b1.jpg" ),
                 testing::Values( 3, 5 ),
                 testing::Bool(),
                 testing::Values( make_tuple(50.0, 100.0), make_tuple(0.0, 50.0) )
             )
           )
{
    String filename = getDataPath(get<0>(GetParam()));
    int aperture = get<1>(GetParam());
    bool useL2 = get<2>(GetParam());
    double thresh_low = get<0>(get<3>(GetParam()));
    double thresh_high = get<1>(get<3>(GetParam()));

    Mat img = imread(filename, IMREAD_GRAYSCALE);
    if (img.empty())
        FAIL() << "Unable to load source image " << filename;
    Mat edges(img.size(), img.type());

    declare.in(img).out(edges);


    TEST_CYCLE(100) { Canny(img, edges, thresh_low, thresh_high, aperture, useL2); }


    SANITY_CHECK(edges);
}
