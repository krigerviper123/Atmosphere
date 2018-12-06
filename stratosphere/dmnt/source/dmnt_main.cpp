/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <malloc.h>

#include <switch.h>
#include <atmosphere.h>
#include <stratosphere.hpp>

#include "dmnt_debug_monitor.hpp"

extern "C" {
    extern u32 __start__;

    u32 __nx_applet_type = AppletType_None;

    #define INNER_HEAP_SIZE 0x80000
    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];
    
    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}


void __libnx_initheap(void) {
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	/* Newlib */
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void __appInit(void) {
    Result rc;
    
    rc = smInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }
    
    rc = pmdmntInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    rc = ldrDmntInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    /*
    if (kernelAbove300()) {
        rc = roDmntInitialize();
        if (R_FAILED(rc)) {
            fatalSimple(rc);
        }
    }
    */
    
    rc = nsdevInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    rc = lrInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    rc = setInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    rc = fsInitialize();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        fatalSimple(rc);
    }
    
    CheckAtmosphereVersion(CURRENT_ATMOSPHERE_VERSION);
}

void __appExit(void) {
    /* Cleanup services. */
    fsdevUnmountAll();
    fsExit();
    setExit();
    lrExit();
    nsdevExit();
    /* if (kernelAbove300()) { roDmntExit(); } */
    ldrDmntExit();
    pmdmntExit();
    smExit();
}

int main(int argc, char **argv)
{
    consoleDebugInit(debugDevice_SVC);
    
    /* Nintendo uses four threads. */
    auto server_manager = new WaitableManager(4);
    
    /* Create services. */
    server_manager->AddWaitable(new ServiceServer<DebugMonitorService>("dmnt:-", 4));

    /* Loop forever, servicing our services. */
    server_manager->Process();

    delete server_manager;

    return 0;
}
