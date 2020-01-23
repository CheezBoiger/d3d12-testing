//
#pragma once

#include "WinConfigs.h"

class Time
{
public:
    static void initialize();
    static void update();

    Time()
        : m_time((R64)g_currT)
        , m_dt(g_deltaT) { }

    R64 dt() const { return m_dt; }
    R64 getTimeStamp() const { return m_time; }

private:

    R64 m_time;
    R64 m_dt;

    static I64 g_currT;
    static I64 g_prevT;
    static R64 g_deltaT;
};