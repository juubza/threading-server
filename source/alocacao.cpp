#include <chrono>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // Para usar atoi

class Timer
{
public:
    Timer()
    {
        start = clock.now();
    }
    double GetElapsed()
    {
        auto end = clock.now();
        auto duration = end - start;
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() * 1.e-9;
    }
private:
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock clock;

    Timer(const Timer&) = delete;
    Timer operator=(const Timer*) = delete;
};

void BusyWait(int ms)
{
    auto end = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
    for (;;)
    {
        if (std::chrono::steady_clock::now() > end)
            break;
    }
}

void FastMeasure(int bufSize)
{
    printf("Busy waiting to raise the CPU frequency...\n");
    BusyWait(500);
    const int iterationCount = 2000;

    printf("\nTesting buffer size: %d MB\n", bufSize / (1024 * 1024));

    {
        Timer timer;
        for (int i = 0; i < iterationCount; ++i)
        {
            int* p = new int[bufSize / sizeof(int)];
            delete[] p;
        }
        printf("%1.4f s to allocate %d MB %d times.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount);
    }
    {
        Timer timer;
        double deleteTime = 0.0;
        for (int i = 0; i < iterationCount; ++i)
        {
            int* p = new int[bufSize / sizeof(int)];
            Timer deleteTimer;
            delete[] p;
            deleteTime += deleteTimer.GetElapsed();
        }
        printf("%1.4f s to allocate %d MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, deleteTime);
    }
    {
        int* p = new int[bufSize / sizeof(int)]();
        {
            Timer timer;
            for (int i = 0; i < iterationCount; ++i)
            {
                memset(p, 1, bufSize);
            }
            printf("%1.4f s to write %d MB %d times.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount);
        }
        {
            Timer timer;
            int sum = 0;
            for (int i = 0; i < iterationCount; ++i)
            {
                for (size_t index = 0; index < bufSize / sizeof(int); ++index)
                {
                    sum += p[index];
                }
            }
            printf("%1.4f s to read %d MB %d times, sum = %d.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, sum);
        }
        delete[] p;
    }
    {
        Timer timer;
        double deleteTime = 0.0;
        for (int i = 0; i < iterationCount; ++i)
        {
            int* p = new int[bufSize / sizeof(int)];
            memset(p, 10, bufSize);
            Timer deleteTimer;
            delete[] p;
            deleteTime += deleteTimer.GetElapsed();
        }
        printf("%1.4f s to allocate and write %d MB %d times (%1.4f s to delete).\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, deleteTime);
    }
    {
        Timer timer;
        int sum = 0;
        for (int i = 0; i < iterationCount; ++i)
        {
            int* p = new int[bufSize / sizeof(int)];
            for (size_t index = 0; index < bufSize / sizeof(int); ++index)
            {
                sum += p[index];
            }
            delete[] p;
        }
        printf("%1.4f s to allocate and read %d MB %d times, sum = %d.\n", timer.GetElapsed(), bufSize / (1024 * 1024), iterationCount, sum);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <buffer size in MB>\n", argv[0]);
        return 1;
    }

    int bufSize = atoi(argv[1]) * 1024 * 1024;
    FastMeasure(bufSize);

    return 0;
}
