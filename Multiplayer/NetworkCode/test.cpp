#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
std::thread t;

void test()
{
    std::cout << "Line 13" << std::endl;
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Line 15" << std::endl;
    cv.wait(lock);
    std::cout << "Test Success\n" << std::endl;
    
}

int main()
{
    std::cout << "Line 23" << std::endl;
    t = std::thread(test);
    std::cout << "Line 25" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Line 27" << std::endl;
    cv.notify_one();
    std::cout << "Line 29" << std::endl;
    t.join();
    std::cout << "Line 31" << std::endl;
    return 0;
}


