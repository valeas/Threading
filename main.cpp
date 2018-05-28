#include <iostream>
#include <lock_guard.h>
#include <thread>
#include <vector>
#include <condition_variable>
#include <stdexcept>      // std::logic_error
#include <chrono>
#include <stdlib.h>

#include <mutex> //already in lock_guard.h, but why not. Usefull as a contingency.

using namespace std;


mutex mtx;  // protects g_i

mutex prot;
int totalTime; //global variable to show protection



//task 2
//c++ reference lock_guard example as test
void print_even (int x) {
  if (x%2==0) std::cout << x << " is even\n";
  else throw (std::logic_error("not even"));
}


void print_thread_id (int id) {
  try {
    // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
    myLock::lock_guard<std::mutex> lck (mtx);
    print_even(id);
  }
  catch (std::logic_error&) {
    std::cout << "[exception caught]\n";
  }
}

struct Counter {
    int value;
    Counter() : value(0){}

    void increment(){
        ++value;
    }
};

struct ConcurrentSafeCounter {
    std::mutex mutex;
    Counter counter;

    void increment(){
        myLock::lock_guard<std::mutex> guard(mutex);
        counter.increment();
    }

    int value(){
        return counter.value;
    }
};


void mutexLockTest(bool correct){
    Counter counter;
    ConcurrentSafeCounter counterS;

        std::vector<std::thread> threads;
        for(int i = 0; i < 5; ++i){
            threads.push_back(std::thread([&counter, &correct, &counterS](){
                for(int i = 0; i < 100000; ++i){
                    if (correct){
                        counterS.increment();
                    } else{
                    counter.increment();
                    }
                }
            }));
        }

        for(auto& thread : threads){
            thread.join();
        }
        if (correct){
            cout <<"With lock: "<< counterS.value() << std::endl; //must be correct
        } else
        cout <<"Without lock: "<< counter.value << std::endl; //unlikely to be correct
}


void lockTest(){
    thread threads[10];
      // spawn 10 threads:
      for (int i=0; i<10; ++i)
        threads[i] = std::thread(print_thread_id,i+1);

      for (auto& th : threads) th.join();

      mutexLockTest(false);
      mutexLockTest(true);
}


// end task 2



//task 3
//some of the time, a display artefact(caused by a race condition) occurs to show that the threads are running concurrently
void thread_function()
{
    for (int i = -10; i < 0; i++)
        cout << "thread function: " << i << endl;
    cout << "thread number " << this_thread::get_id() << endl;
}

void newThreads(){
    vector<thread> threads;

    for(int i = 0; i < 3; ++i){
            threads.push_back(thread(thread_function));
        }

        for(auto& myThread : threads){
            myThread.join();
        }
}



//task 5

void addToTime(int timeElapsed){
    myLock::lock_guard<std::mutex> lock (prot);
    totalTime+=timeElapsed;
    cout<< "Current thread: " << this_thread::get_id() << endl;
    cout<< "Total time: "<< totalTime <<endl;
}

std::mutex m;
std::mutex startTest;
std::string data;
bool ready = false;
bool processed = false;
condition_variable cv;
condition_variable start;
int following=0;
vector<condition_variable> cvector(3);

//task 4
//first atempts
void thread_action(int i)
{
    unique_lock<std::mutex> lk(m);
    int time;
    cout<< "Thread "<<i<<" is ready"<<endl;
    start.notify_one(); //thread is ready
    while(1){
        cvector[(i+1)%3].notify_one();
        cvector[i].wait(lk);
        time= rand() % 5 + 1;
        std::cout << "Thread "<<i<<" is sleeping for "<< time << " seconds." <<endl;
        this_thread::sleep_for(chrono::milliseconds(time*1000));
        cout<<"passing argument to next thread"<<endl;
        addToTime(time);
        cout<<"-----------------"<<endl;
        lk.unlock();
    }
}

//original attempt, kept for reference
void synchronizedThreads(){
    //condition_variable cv;
    cout<<"Task 4"<<endl;
    vector<thread> threads;
        for(int i = 0; i < 3; ++i){
            threads.push_back(thread(thread_action, i));
        }

   int completed=0;
   while(completed<3){
      cv.notify_one();
      completed+=1;
   }
    //start the threads
    cvector[0].notify_one();

        for(auto& myThread : threads){
            myThread.join();
        }
}

void dataValues(int i){
    int time= rand() % 5 + 1;
    std::cout << "Thread "<<i<<" is sleeping for "<< time << " seconds." <<endl;
    this_thread::sleep_for(chrono::milliseconds(time*1000));
    cout<<"passing argument to next thread"<<endl;
    addToTime(time);
    cout<<"-----------------"<<endl;
}

void thread_action_new(int i)
{
    unique_lock<std::mutex> lk(m);
    int time;
    cout<< "Thread "<<i<<" is ready"<<endl;


    start.notify_one(); //thread is ready
    cvector[i].wait(lk);
    while(1){
        dataValues(i);

        cvector[(i+1)%3].notify_one();
        cvector[i].wait(lk);
    }
}



void synchronizedFinal(){
    cout<<"Task 4"<<endl;
    vector<thread> threads;
    unique_lock<std::mutex> startLock(startTest);
        for(int i = 0; i < 3; ++i){
            threads.push_back(thread(thread_action_new, i));
            start.wait(startLock);
            cout<<"going for next"<<endl;
        }


    //start the threads
        cout<<"starting cycle"<<endl;
    cvector[0].notify_one();

        for(auto& myThread : threads){
            myThread.join();
        }
        cout<<"!!!"<<endl;
}

//uncomment for compiling
/*int main()
{
    //lock_guard test from c++ reference
   lockTest();

    //end lock guard test

   //task 3
   newThreads();

   //task 4
   //std::condition_signal does not exist
   //using std::condition_variable, should be what was intended
   //synchronizedThreads();
   synchronizedFinal();

    cout << "works";
    return 0;
}*/
