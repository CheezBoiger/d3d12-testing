// 
#include "Time.h"


I64 Time::g_currT = 0;
R64 Time::g_deltaT = 0.0;
I64 Time::g_prevT = 0;

I64 g_ticksPerTime = 0;


void Time::initialize()
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_ticksPerTime)) {
    }

    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_prevT)) {

    }

    update();
}


void Time::update()
{
    QueryPerformanceCounter((LARGE_INTEGER *)&g_currT);
    g_deltaT = (R64)(g_currT - g_prevT) / (R64)g_ticksPerTime;
    g_prevT = g_currT;
}