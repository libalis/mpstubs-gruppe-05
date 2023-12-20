#include "machine/context.h"
#include "debug/output.h"

#define NON_SCRTCH_REG_CNT 6

void context_panic() {
	DBG << "Application should not return!1!!11";
	DBG.flush();
	Core::die();
}

void * prepareContext(void * tos, void (*kickoff)(void *),
                      void * param1) {
	void** rsp = reinterpret_cast<void**>(tos);
	rsp--;
	*rsp = reinterpret_cast<void*>(context_panic);
	rsp--;
	*rsp = reinterpret_cast<void*>(kickoff);
	rsp--;
	*rsp = reinterpret_cast<void*>(prepare_parameter);
	rsp--;
	*rsp = reinterpret_cast<void*>(param1);
	for (uint8_t i = 0; i < NON_SCRTCH_REG_CNT - 1; i++) {
		rsp--;
		*rsp = reinterpret_cast<void*>(0);
	}
	return rsp;
}
