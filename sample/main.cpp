//#define FULL_DISABLE_PROFILER
#include <chrono>
#include <thread>
#include <vector>
#include <iostream>
#include <condition_variable>
#include <cstdlib>
#include <math.h>

#include <easy/profiler.h>
#include <easy/reader.h>

std::condition_variable cv;
std::mutex cv_m;
int g_i = 0;

int OBJECTS = 500;
int MODELLING_STEPS = 1500;
int RENDER_STEPS = 1500;
int RESOURCE_LOADING_COUNT = 50;

//#define SAMPLE_NETWORK_TEST

void localSleep(int magic=200000)
{
    //PROFILER_BEGIN_FUNCTION_BLOCK_GROUPED(profiler::colors::Blue);
    volatile int i = 0;
    for (; i < magic; ++i);
}

void loadingResources(){
    EASY_FUNCTION(profiler::colors::DarkCyan);
    localSleep();
//    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void prepareMath(){
    EASY_FUNCTION(profiler::colors::Green);
    int* intarray = new int[OBJECTS];
    for (int i = 0; i < OBJECTS; ++i)
        intarray[i] = i * i;
    delete[] intarray;
    //std::this_thread::sleep_for(std::chrono::milliseconds(3));
}

void calcIntersect(){
    EASY_FUNCTION(profiler::colors::Gold);
    //int* intarray = new int[OBJECTS * OBJECTS];
    int* intarray = new int[OBJECTS];
    for (int i = 0; i < OBJECTS; ++i)
    {
        for (int j = i; j < OBJECTS; ++j)
            //intarray[i * OBJECTS + j] = i * j - i / 2 + (OBJECTS - j) * 5;
            intarray[j] = i * j - i / 2 + (OBJECTS - j) * 5;
    }
    delete[] intarray;
    //std::this_thread::sleep_for(std::chrono::milliseconds(4));
}

double multModel(double i)
{
    EASY_FUNCTION(profiler::colors::PaleGold);
    return i * sin(i) * cos(i);
}

void calcPhys(){
    EASY_FUNCTION(profiler::colors::Amber);
    double* intarray = new double[OBJECTS];
    for (int i = 0; i < OBJECTS; ++i)
        intarray[i] = multModel(double(i)) + double(i / 3) - double((OBJECTS - i) / 2);
    calcIntersect();
    delete[] intarray;
}

double calcSubbrain(int i)
{
    EASY_FUNCTION(profiler::colors::Navy);
    return i * i * i - i / 10 + (OBJECTS - i) * 7 ;
}

void calcBrain(){
    EASY_FUNCTION(profiler::colors::LightBlue);
    double* intarray = new double[OBJECTS];
    for (int i = 0; i < OBJECTS; ++i)
        intarray[i] = calcSubbrain(i) + double(i * 180 / 3);
    delete[] intarray;
    //std::this_thread::sleep_for(std::chrono::milliseconds(3));
}

void calculateBehavior(){
    EASY_FUNCTION(profiler::colors::Blue);
    calcPhys();
    calcBrain();
}

void modellingStep(){
    EASY_FUNCTION();
    prepareMath();
    calculateBehavior();
}

void prepareRender(){
    EASY_FUNCTION(profiler::colors::Brick);
    localSleep();
    //std::this_thread::sleep_for(std::chrono::milliseconds(8));

}

int multPhys(int i)
{
    EASY_FUNCTION(profiler::colors::Red700, profiler::ON);
    return i * i * i * i / 100;
}

int calcPhysicForObject(int i)
{
    EASY_FUNCTION(profiler::colors::Red);
    return  multPhys(i) + i / 3 - (OBJECTS - i) * 15;
}

void calculatePhysics(){
    EASY_FUNCTION(profiler::colors::Red);
    unsigned int* intarray = new unsigned int[OBJECTS];
    for (int i = 0; i < OBJECTS; ++i)
        intarray[i] = calcPhysicForObject(i);
    delete[] intarray;
    //std::this_thread::sleep_for(std::chrono::milliseconds(8));
}

void frame(){
    EASY_FUNCTION(profiler::colors::Magenta);
    prepareRender();
    calculatePhysics();
}

void loadingResourcesThread(){
    //std::unique_lock<std::mutex> lk(cv_m);
    //cv.wait(lk, []{return g_i == 1; });
    EASY_THREAD("Resource loading");
#ifdef SAMPLE_NETWORK_TEST
    while (true) {
#else
    for(int i = 0; i < RESOURCE_LOADING_COUNT; i++){
#endif
        loadingResources();
        EASY_EVENT("Resources Loading!", profiler::colors::Cyan);
        localSleep(1200000);
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void modellingThread(){
    //std::unique_lock<std::mutex> lk(cv_m);
    //cv.wait(lk, []{return g_i == 1; });
    EASY_THREAD("Modelling");
#ifdef SAMPLE_NETWORK_TEST
    while (true) {
#else
    for (int i = 0; i < MODELLING_STEPS; i++){
#endif
        EASY_FRAME_COUNTER;
        modellingStep();
        EASY_END_FRAME_COUNTER;

        localSleep(1200000);
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void renderThread(){
    //std::unique_lock<std::mutex> lk(cv_m);
    //cv.wait(lk, []{return g_i == 1; });
    EASY_THREAD("Render");
#ifdef SAMPLE_NETWORK_TEST
    while (true) {
#else
    for (int i = 0; i < RENDER_STEPS; i++){
#endif
        frame();
        localSleep(1200000);
        //std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    if (argc > 1 && argv[1]){
        OBJECTS = std::atoi(argv[1]);
    }
    if (argc > 2 && argv[2]){
        MODELLING_STEPS = std::atoi(argv[2]);
    }
    if (argc > 3 && argv[3]){
        RENDER_STEPS = std::atoi(argv[3]);
    }
    if (argc > 4 && argv[4]){
        RESOURCE_LOADING_COUNT = std::atoi(argv[4]);
    }

    std::cout << "Objects count: " << OBJECTS << std::endl;
    std::cout << "Render steps: " << MODELLING_STEPS << std::endl;
    std::cout << "Modelling steps: " << RENDER_STEPS << std::endl;
    std::cout << "Resource loading count: " << RESOURCE_LOADING_COUNT << std::endl;

    auto start = std::chrono::system_clock::now();

#ifndef SAMPLE_NETWORK_TEST
    EASY_PROFILER_ENABLE;
#endif

    EASY_MAIN_THREAD;
    profiler::startListen();

    std::vector<std::thread> threads;
    //for (int i=0; i < 3; i++)
    {
        threads.emplace_back(loadingResourcesThread);
        threads.emplace_back(renderThread);
        threads.emplace_back(modellingThread);
    }

    cv_m.lock();
    g_i = 1;
    cv_m.unlock();
    cv.notify_all();

    std::atomic_bool stop = ATOMIC_VAR_INIT(false);
    auto ttt = std::thread([&stop]()
    {
        while (!stop.load(std::memory_order_acquire))
        {
            std::cout << "Frame time: " << profiler::main_thread::frameTimeLocalMax(profiler::MICROSECONDS) << " us\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    modellingThread();

    stop.store(true, std::memory_order_release);

    for(auto& t : threads)
        t.join();

    ttt.join();

    auto end = std::chrono::system_clock::now();
    auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Elapsed time: " << elapsed.count() << " usec" << std::endl;

    auto blocks_count = profiler::dumpBlocksToFile("test.prof");

    std::cout << "Blocks count: " << blocks_count << std::endl;

    return 0;
}
