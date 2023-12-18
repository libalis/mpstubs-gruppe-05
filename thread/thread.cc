#include "thread/thread.h"
#include "thread/dispatcher.h"

uint8_t the_sarlacc_pit[Thread::STACK_SIZE];
size_t count;

#include "debug/output.h"

void Thread::go() {
    void** rsp = reinterpret_cast<void**>(&the_sarlacc_pit[STACK_SIZE - 1]);
    StackPointer sp;
    sp.kernel = rsp;
    context_switch(sp, stackpointer);
}

void Thread::kickoff(void* object) {  // !!!TODO!!!
    kickoff(reinterpret_cast<Thread*>(object));
}

void Thread::kickoff(Thread* object) {
    object->action();
}

void Thread::resume(Thread* next) {
    context_switch(stackpointer, next->stackpointer);
}

Thread::Thread() : id(count++) {
    reserved_stack_space[1] = 0xaa;
    reserved_stack_space[0] = 0x55;
    stackpointer.kernel = prepareContext(&reserved_stack_space[STACK_SIZE - 1], kickoff, this);
}
