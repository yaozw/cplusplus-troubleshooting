#include <pthread.h>
#include <iostream>
#include <queue>
using namespace std;

#include "stacktrace.h"
#include <signal.h>

/* 
  This program usually will crash because queue push and pop is not
  protected by lock, when it crash, call trace will print it.
  To compile:

  g++ -o th thread.cpp  -rdynamic  -l pthread;

*/

queue<long> ql;

bool p_quit = false;

static void signalHandler(int theSignal,siginfo_t *theSip,void *theContext)
{
    print_stacktrace();
}

volatile long count = 0;
long MAX = 100000000;
long popCount = 0;

static void *producer(void *arg)
{
    while (count < MAX)
    {
        ql.push(count++);
    }
    p_quit = true;
}

static void *consumer(void *arg)
{
    int i;
    while (!p_quit)
    {
        while (ql.size() > 0)
        {
            ql.pop();
            popCount++;
        }
    }
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    struct sigaction oldAct;
    act.sa_handler = 0;
    act.sa_sigaction = signalHandler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGBUS);
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGABRT, &act, &oldAct) < 0)
    {
        cout << "ExceptionHandler::ExceptionHandler() <sigaction(SIGABRT)>" ;
    }

    if (sigaction(SIGSEGV, &act, &oldAct) < 0)
    {
        cout << "ExceptionHandler::ExceptionHandler() <sigaction(SIGSEGV)>";
    }

    // +++ TESTRT
    if (sigaction(SIGUSR2, &act, &oldAct) < 0)
    {
        cout << "ExceptionHandler::ExceptionHandler() <sigaction(SIGUSR2)>";
    }
    // --- TESTRT

    int s, tnum, opt, num_threads;
    struct thread_info *tinfo;
    pthread_attr_t p_attr;
    pthread_attr_t c_attr;
    int stack_size;
    void *res;
    pthread_t p_thread;
    pthread_t c_thread;

    s = pthread_attr_init(&p_attr);
    if (s != 0)
        cout << "init err";
    s = pthread_attr_init(&c_attr);
    if (s != 0)
        cout << "init err";

    s = pthread_create(&p_thread, &p_attr,
                       &producer, (void *)1);
    if (s != 0)
        cout << "create err";
    s = pthread_create(&c_thread, &c_attr,
                       &consumer, (void *)1);
    if (s != 0)
        cout << "create err";

    int rc = pthread_join(p_thread, NULL);
    rc = pthread_join(c_thread, NULL);
    cout << "produce:" << count << endl;
    cout << popCount << endl;
    cout << count - popCount << endl;
    cout << "main quit" << endl;
    return 0;
}
