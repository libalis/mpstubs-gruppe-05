#include "machine/context.h"

void * prepareContext(void * tos, void (*kickoff)(void *),
                      void * param1) {
	(void) tos;
	(void) kickoff;
	(void) param1;
	return nullptr;
}
