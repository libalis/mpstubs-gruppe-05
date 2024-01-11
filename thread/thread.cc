#include "thread/thread.h"
#include "interrupt/guard.h"

static size_t count;

void Thread::go() {
    StackPointer sp;
    context_switch(sp, stackpointer);
}

void Thread::kickoff(Thread* object) {
    Guard::leave();
    object->action();
}

void Thread::resume(Thread* next) {
    assert(reserved_stack_space[1] == 0xaa && reserved_stack_space[0] == 0x55);
    context_switch(stackpointer, next->stackpointer);
}

Thread::Thread() : id(count++) {
    reserved_stack_space[1] = 0xaa;
    reserved_stack_space[0] = 0x55;
    void (* casted_kickoff)(void *);
    casted_kickoff = reinterpret_cast<void (*)(void *)>(kickoff);
    stackpointer.kernel = prepareContext(&reserved_stack_space[STACK_SIZE - 8], casted_kickoff, this);
}
