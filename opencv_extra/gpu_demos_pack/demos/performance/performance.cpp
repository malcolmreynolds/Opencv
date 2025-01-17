#include <iomanip>
#include <stdexcept>
#include <string>
#include "performance.h"

using namespace std;
using namespace cv;

void TestSystem::run()
{
    if (is_list_mode_)
    {
        for (vector<Runnable*>::iterator it = tests_.begin(); it != tests_.end(); ++it)
            if ((*it)->name().find(test_filter_, 0) != string::npos)
                cout << (*it)->name() << endl;
        return;
    }

    // Run test initializers    
    for (vector<Runnable*>::iterator it = inits_.begin(); it != inits_.end(); ++it)
    {
        if ((*it)->name().find(test_filter_, 0) != string::npos)
            (*it)->run();
    }

    printHeading();

    // Run tests
    for (vector<Runnable*>::iterator it = tests_.begin(); it != tests_.end(); ++it)
    {
        try
        {
            if ((*it)->name().find(test_filter_, 0) != string::npos)
            {
                cout << endl << (*it)->name() << ":\n";
                (*it)->run();
                finishCurrentSubtest();
            }
        }
        catch (const Exception&)
        {
            // Message is printed via callback
            resetCurrentSubtest();
        }
        catch (const runtime_error& e)
        {
            printError(e.what());
            resetCurrentSubtest();
        }
    }

    printSummary();

    cout << "Press any key to continue...";
    cin.get();
}


void TestSystem::finishCurrentSubtest()
{
    if (cur_subtest_is_empty_)
        // There is no need to print subtest statistics
        return;

    int cpu_time = static_cast<int>(cpu_elapsed_ / getTickFrequency() * 1000.0);
    int gpu_time = static_cast<int>(gpu_elapsed_ / getTickFrequency() * 1000.0);

    double speedup = static_cast<double>(cpu_elapsed_) /
                     std::max((int64)1, gpu_elapsed_);
    speedup_total_ += speedup;

    printMetrics(cpu_time, gpu_time, speedup);
    
    num_subtests_called_++;
    resetCurrentSubtest();
}


void TestSystem::printHeading()
{
    cout << setiosflags(ios_base::left);
    cout << TAB << setw(10) << "CPU, ms" << setw(10) << "GPU, ms" 
        << setw(14) << "SPEEDUP" 
        << "DESCRIPTION\n";
    cout << resetiosflags(ios_base::left);
}


void TestSystem::printSummary()
{
    cout << setiosflags(ios_base::fixed);
    cout << "\naverage GPU speedup: x" 
        << setprecision(3) << speedup_total_ / std::max(1, num_subtests_called_) 
        << endl;
    cout << resetiosflags(ios_base::fixed);
}


void TestSystem::printMetrics(double cpu_time, double gpu_time, double speedup)
{
    cout << TAB << setiosflags(ios_base::left);
    stringstream stream;

    stream << cpu_time;
    cout << setw(10) << stream.str();

    stream.str("");
    stream << gpu_time;
    cout << setw(10) << stream.str();

    stream.str("");
    stream << "x" << setprecision(3) << speedup;
    cout << setw(14) << stream.str();

    cout << cur_subtest_description_.str();
    cout << resetiosflags(ios_base::left) << endl;
}


void TestSystem::printError(const std::string& msg)
{
    cout << TAB << "[error: " << msg << "] " << cur_subtest_description_.str() << endl;
}


void gen(Mat& mat, int rows, int cols, int type, Scalar low, Scalar high)
{
    mat.create(rows, cols, type);
    RNG rng(0);
    rng.fill(mat, RNG::UNIFORM, low, high);
}


string abspath(const string& relpath)
{
    return TestSystem::instance().workingDir() + relpath;
}


int CV_CDECL cvErrorCallback(int /*status*/, const char* /*func_name*/, 
                             const char* err_msg, const char* /*file_name*/,
                             int /*line*/, void* /*userdata*/)
{
    TestSystem::instance().printError(err_msg);
    return 0;
}


int main(int argc, char** argv)
{
    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        string key = argv[i];
        if (key == "--help")
        {
            cout << "Usage: demo_performance [--ls] [--filter <test_filter>]\n";
            return 0;
        }
        if (key == "--filter" && i + 1 < argc)
            TestSystem::instance().setTestFilter(argv[++i]);
        //else if (key == "--workdir" && i + 1 < argc)
        //    TestSystem::instance().setWorkingDir(argv[++i]);
        else if (key == "--ls")
            TestSystem::instance().setListMode(true);
        else 
        {
            cout << "Unknown parameter: '" << key << "'" << endl;
            return -1;
        }
    }

    redirectError(cvErrorCallback);
    TestSystem::instance().run();

    return 0;
}
