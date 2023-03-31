#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include <minix/syslib.h>
#include <sys/ioc_homework.h>
#include "homework.h"


#define NUM_SLOTS 5

static int slot_index = 0;
static int slots[NUM_SLOTS] = {0};

/*
 * Function prototypes for the homework driver.
 */
static int homework_open(devminor_t minor, int access, endpoint_t user_endpt);
static int homework_close(devminor_t minor);
static int homework_read(int fd, int foo, size_t size);
static int homework_write(int fd , int foo, size_t size);
static int homework_ioctl(int fd, unsigned long request, int foo);
 
/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int, int);
static int lu_state_restore(void);
 
/* Entry points to the homework driver. */
static struct chardriver homework_tab =
{
    .cdr_open	= homework_open,
    .cdr_close	= homework_close,
    .cdr_read	= homework_read,
    .cdr_write  = homework_write,
    .cdr_ioctl  = homework_ioctl,
};

static int open_counter;

/* Open function, based off of example. */
static int homework_open(devminor_t UNUSED(minor), int UNUSED(access), endpoint_t UNUSED(user_endpt))
{
    printf("homework_open(). Called %d time(s).\n", ++open_counter");
    return OK;
}

/* Close function, based off example. */ 
static int homework_close(devminor_t UNUSED(minor))
{
    printf("homework_close()\n");
    return OK;
}

/* Read function.*/
static int homework_read(int fd, int foo, size_t size)
{
	printf("Calling READ...\n");
	
	/* Checks if the bytes sent to be read are 4 (the size of a 32bit int). */
	/* If message is too short, returns error. */
	if (size < sizeof(int))
	{
		return(EINVAL);	
	/* Otherwise, continues. 
	 * Reads data from slot_index value.
	 */
    } else {
        int value = slots[slot_index];
        printf("Read %d from slot %d.\n", value, slot_index);
        return 4;
    };
}

/* Write function.*/
static int homework_write(int fd , int foo, size_t size)
{
	printf("homework_write()\n");
	
	/* Checks if the bytes sent to be written are 4 (the size of a 32bit int). */
    if (size < sizeof(int)) 
    {
        return(EINVAL);
	/* Otherwise, continues.  
	 * Writes in data to currently selected slot value.
	 */
    } else {
        int value = foo;
        slots[slot_index] = value;
        printf("Wrote %d to slot %d.\n", slots[slot_index], slot_index);
        return 4;
    }
}

/* Function that receives I/O control call. */    
int homework_ioctl(int fd, unsigned long request, int foo)
{
	printf("homework_ioctl()\n");
	
	int new_slot = 0;
	int current_slot = slot_index;
	
	/* Judges based off of the REQUEST aspect of a message struct. */
	switch (request) {
	
        case HIOCSLOT:
        	/* If message is HIOCSLOT, then checks if requested slot is 0 <= m >= 4. */
            new_slot = foo;
            if (new_slot < 0 || new_slot > 4) {
                printf("Error: Only integers between 0 and 4 are accepted.\n");
                return EINVAL;
            
            /* If slot requested is fine, then sets requested slot as current and returns. */ 
            } else {
            	slot_index = new_slot;
            	printf("Changed current slot to slot %d.\n", slot_index);
            }
            break;
            
        case HIOCCLEARSLOT:
        /* If message is HIOCCLEARSLOT, then sets current slot to 0, sets slot_index to the invalid value of -1. */
            new_slot = foo;
            slots[new_slot] = 0;
            slot_index = -1;
            break;
        
        /* If message is HIOCGETSLOT, then identifies slot requested from message and returns as current. */
        case HIOCGETSLOT:
            if (current_slot < 0 || current_slot > 4) {
                printf("Error: Current slot is invalid, please set a slot value between 0 and 4.\n");
                return EINVAL;
            } else {
            	printf("Current slot is slot %d. \n", current_slot);
            }
            break;
            
        default:
        	printf("That isn't an accepted IOCTL value.\n");
            return EINVAL;
    }
    return OK;
}

static int sef_cb_lu_state_save(int UNUSED(state), int UNUSED(flags)) {
/* Save the state. */
    ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);
 
    return OK;
}

static int lu_state_restore() {
/* Restore the state. */
    u32_t value;
 
    ds_retrieve_u32("open_counter", &value);
    ds_delete_u32("open_counter");
    open_counter = (int) value;
 
    return OK;
}

static void sef_local_startup()
{
    /*
     * Register init callbacks. Use the same function for all event types
     */
    sef_setcb_init_fresh(sef_cb_init);
    sef_setcb_init_lu(sef_cb_init);
    sef_setcb_init_restart(sef_cb_init);
 
    /*
     * Register live update callbacks.
     */
    sef_setcb_lu_state_save(sef_cb_lu_state_save);
 
    /* Let SEF perform startup. */
    sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
/* Initialize the homework driver. */
    int do_announce_driver = TRUE;
 
 	open_counter = 0;
    switch(type) {
        case SEF_INIT_FRESH:
            printf("%s", HOMEWORK_MESSAGE);
        break;
 
        case SEF_INIT_LU:
            /* Restore the state. */
            lu_state_restore();
            do_announce_driver = FALSE;
 
            printf("%sHey, I'm a new version!\n", HOMEWORK_MESSAGE);
        break;
 
        case SEF_INIT_RESTART:
            printf("%sHey, I've just been restarted!\n", HOMEWORK_MESSAGE);
        break;
    }
 
    /* Announce we are up when necessary. */
    if (do_announce_driver) {
        chardriver_announce();
    }
 
    /* Initialization completed successfully. */
    return OK;
}

int main(void)
{
    /*
     * Perform initialization.
     */
    sef_local_startup();
 
    /*
     * Run the main loop.
     */
    chardriver_task(&homework_tab);
    return OK;
}