/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * jitc.c
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include "system.h"
#include "jitc.h"

/**
 * Needs:
 *   fork()
 *   execv()
 *   waitpid()
 *   WIFEXITED()
 *   WEXITSTATUS()
 *   dlopen()
 *   dlclose()
 *   dlsym()
 */

/* research the above Needed API and design accordingly */
struct jitc {
	void * handle;
};

int jitc_compile(const char *input, const char *output) {
    pid_t process_id = fork(); 
    /* process_id = 0 for child, positive for parent, and will -1 indicate fork failure */
    
    if (process_id == 0) {  /* if child process */
        /* Child process - invoke gcc to compile the .c file and generate the .so shared object */
        char *args[7];
        args[0] = "gcc";
        args[1] = "-shared";
        args[2] = "-o";
        args[3] = (char *)output;
        args[4] = "-fPIC";
        args[5] = (char *)input;
        args[6] = NULL;
        
        execv("/usr/bin/gcc", args);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
        return -1;
    } 
    else if (process_id > 0) {
        /* Parent process - wait for the child to complete execution */
        int child_status;
        waitpid(process_id, &child_status, 0);
        if (WIFEXITED(child_status)) {
            int exit_code = WEXITSTATUS(child_status);        
            printf("Child process exited with code %d\n", exit_code);
        }
        return 0;
    } 
    else {
        printf("Error: fork() failed!\n");
        return -1;
    }
}


struct jitc *jitc_open(const char *pathname) {
    void *lib_handle;
    struct jitc *jitc_ptr;
    
    /* printf("Provided file path: %s\n", pathname); */

    lib_handle = dlopen(pathname, RTLD_LAZY);
    jitc_ptr = (struct jitc *)malloc(sizeof(struct jitc));
    if (lib_handle == NULL) {
        free(jitc_ptr);
        return NULL;
    }
    
    jitc_ptr->handle = lib_handle;
    return jitc_ptr;
}


void jitc_close(struct jitc *jitc) {
    if (dlclose(jitc->handle) == 0) 
        {
        printf("Successfully closed the handle\n");
        } 
    else 
        {
        printf("Failed to close the handle\n");
        }
    free(jitc);
}


long jitc_lookup(struct jitc *jitc, const char *symbol) {
    long sym_address = (long)dlsym(jitc->handle, symbol);

    if (sym_address == 0) {
        printf("Unable to locate symbol: %s\n", symbol);
        return 0;  /* Error case handled appropriately */
    }
    
    return sym_address;
}