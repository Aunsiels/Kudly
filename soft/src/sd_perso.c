#include "sd_perso.h"
#include "ff.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"

#define POLLING_INTERVAL                10
#define POLLING_DELAY                   10

/*
 * Find the last / before the given position
 */

static char * findLastSlash (char * begin, char * end) {
    char * pos = begin;
    char * next = begin;
    while((next = strchr(next, '/')) != NULL && next < end){
        pos = next;
    }
    return pos;
}

/*
 * Transform a path to a condensed one, i.e. without ./ and ../
 */
static void transformPath (BaseSequentialStream *chp, char * path) {
    int length = strlen(path);
    char * lastslash = path;
    while(lastslash - path < length){
        /* Find the file */
        char * nextslash = strchr(lastslash + 1, '/');
        /* End of the string found without / */
        if (nextslash == NULL) return;
        /* If we stay in the same directory */
        if( nextslash - lastslash == 2 && lastslash[1] == '.'){
            strcpy(lastslash, nextslash);
            length -= 2;
        /* if we go to the former directory */
        } else if (nextslash - lastslash == 3 && lastslash[1] == '.'
            && lastslash[2] == '.'){
            lastslash = findLastSlash(path, lastslash);
            length -= nextslash - lastslash; 
            strcpy(lastslash,nextslash);
        } else {
            lastslash = nextslash;
        }
    }
    chprintf(chp, "Path : %s\r\n", path);
}

/*
 * Current directory
 */

static char current_dir[1024];

/*
 *  Card monitor timer.
 */
static VirtualTimer tmr;

/*
 * Debounce counter.
 */
static unsigned cnt;

/*
 * Card event sources.
 */
static EventSource inserted_event, removed_event;

/*
 * Insertion monitor timer callback function.
 *
 * p is a pointer to an object implementing @p BaseBlockDevice
 */
static void tmrfunc(void *p) {
    BaseBlockDevice *bbdp = p;
  
    /* The presence check is performed only while the driver is not in a
       transfer state because it is often performed by changing the mode of
       the pin connected to the CS/D3 contact of the card, this could disturb
       the transfer.*/
    blkstate_t state = blkGetDriverState(bbdp);
    chSysLockFromIsr();
    if ((state != BLK_READING) && (state != BLK_WRITING)) {
        /* Safe to perform the check.*/
        if (cnt > 0) {
            if (blkIsInserted(bbdp)) {
                /* Makes sure there was no debounce */
                if (--cnt == 0) {
                    chEvtBroadcastI(&inserted_event);
                }
            } else
                /* Initializes the debouncing counter */
                cnt = POLLING_INTERVAL;
        } else {
            /* Was the sd removed ? */
            if (!blkIsInserted(bbdp)) {
                cnt = POLLING_INTERVAL;
                chEvtBroadcastI(&removed_event);
            }
        }
    }
    /* Calls the function again after a polling delay */
    chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, bbdp);
    chSysUnlockFromIsr();
}

/**
 * Polling monitor start.
 *
 * p is a pointer to an object implementing @p BaseBlockDevice
 */
static void tmr_init(void *p) {
    /* Initializes insert and remove events */
    chEvtInit(&inserted_event);
    chEvtInit(&removed_event);
    chSysLock();
    /* Initializes the polling counter */
    cnt = POLLING_INTERVAL;
    chVTSetI(&tmr, MS2ST(POLLING_DELAY), tmrfunc, p);
    chSysUnlock();
}

/*
 * FS object.
 */
FATFS MMC_FS;

/*
 * MMC driver instance.
 */
MMCDriver MMCD1;

/* FS mounted and ready.*/
static bool_t fs_ready = FALSE;

/* Maximum speed SPI configuration (18MHz, CPHA=0, CPOL=0, MSb first).*/
static SPIConfig hs_spicfg = {NULL, GPIOD, 10, 0};

/* Low speed SPI configuration (281.250kHz, CPHA=0, CPOL=0, MSb first).*/
static SPIConfig ls_spicfg = {NULL, GPIOD, 10,
                              SPI_CR1_BR_2 | SPI_CR1_BR_1};

/* MMC/SD over SPI driver configuration.*/
static MMCConfig mmccfg = {&SPID2, &ls_spicfg, &hs_spicfg};

/* Generic large buffer.*/
uint8_t fbuff[1024];

/* Scans all the files */
FRESULT scan_files(BaseSequentialStream *chp, char *path) {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;

/*Long File Name*/
#if _USE_LFN
    fno.lfname = 0;
    fno.lfsize = 0;
#endif
    /* Open the directory given in the path */
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            /* The next directory to explore */
            res = f_readdir(&dir, &fno);
            /* If nothing, then stop */
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            /* Current directorty */
            if (fno.fname[0] == '.')
                continue;
            /* Takes the name */
            fn = fno.fname;
            /* If it is a dir */
            if (fno.fattrib & AM_DIR) {
                path[i++] = '/';
                strcpy(&path[i], fn);
                /* Reads the files in this dir */
                res = scan_files(chp, path);
                if (res != FR_OK)
                    break;
                /* Removes the end of the path*/
                path[--i] = 0;
            }
            else {
                /* Prints a normal file */
                chprintf(chp, "%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

/* Scans the files in a dir*/
FRESULT ls(BaseSequentialStream *chp, char *path) {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;

/*Long File Name*/
#if _USE_LFN
    fno.lfname = 0;
    fno.lfsize = 0;
#endif
    /* Open the directory given in the path */
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        for (;;) {
            /* The next directory to explore */
            res = f_readdir(&dir, &fno);
            /* If nothing, then stop */
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            /* Current directorty */
            if (fno.fname[0] == '.')
                continue;
            /* Takes the name */
            fn = fno.fname;
            /* If it is a dir */
            if (fno.fattrib & AM_DIR) {
                chprintf(chp, "%s/%s/\r\n", path, fn);
            }else {
                /* Prints a normal file */
                chprintf(chp, "%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

/* Print a given file*/
FRESULT cat(BaseSequentialStream *chp, char *path) {
    FRESULT res;
    FIL fil;       /* File object */

    res = f_open(&fil, path, FA_READ);
    if (res) return res;

    /* Read all lines and display it */
    while (f_gets((TCHAR *)fbuff, sizeof(fbuff), &fil))
        chprintf(chp,(char *)fbuff);

    /* Close the file */
    f_close(&fil);

    return 0;
}

/*
 * cat command for the shell
 */
void cmdCat(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        chprintf(chp, "Usage: cat file\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    if (argv[0][0] == '/') {
        cat(chp, argv[0]);
    }else {
        /* Initialization */
        int length = strlen(current_dir);
        if(current_dir[length-1] != '/'){
            current_dir[length] = '/';
            length++;
        }
        strcpy(current_dir+length, argv[0]);
        cat(chp, current_dir);
        /* Clear */
        if (current_dir[length-1] == '/'){
            current_dir[length-1] = 0;
        } else {
            current_dir[length] = 0;
        }
    }
}

/*
 * Ls command for the shell
 */
void cmdLs(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: ls\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    /* Prints the files */
    ls(chp, current_dir);
}

/*
 * pwd command for the shell
 */
void cmdPwd(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: pwd\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    chprintf(chp, "Current directory : %s\r\n", current_dir);
}

/*
 * cd command for the shell.
 */
void cmdCd(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    int length = 0;
    if (argc > 1) {
        chprintf(chp, "Usage: cd or cd path\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    if (argc == 0){
        current_dir[0] = 0;
    }else {
        if(argv[0][0] == '/'){
            strcpy(current_dir, argv[0]);
        }else {
            length = strlen(current_dir);
            if (length != 0 && current_dir[length-1] != '/'){
                current_dir[length] = '/';
                ++length;
            }
            strcpy(current_dir+length,argv[0]);
            length = strlen(current_dir);
            if (length != 0 && current_dir[length-1] != '/'){
                current_dir[length] = '/';
                current_dir[length+1] = 0;
            }
                
        }
        transformPath(chp, current_dir);
        length = strlen(current_dir);
        /* Remove the / */
        if (length != 0 && current_dir[length-1] == '/'){
            current_dir[length-1] = 0;
        }
        chprintf(chp, "Current directory : %s\r\n", current_dir);
        /* TODO : check if it is a directory ? */
    }
}

/*
 * MMC card insertion event.
 */
static void InsertHandler(eventid_t id) {
    FRESULT err;
  
    (void)id;
    /*
     * On insertion MMC initialization and FS mount.
     */
    if (mmcConnect(&MMCD1)) {
        return;
    }
    /* Mounts the sd card */
    err = f_mount(0, &MMC_FS);
    if (err != FR_OK) {
        mmcDisconnect(&MMCD1);
        return;
    }
    fs_ready = TRUE;
}

/*
 * MMC card removal event.
 */
static void RemoveHandler(eventid_t id) {

    (void)id;
    mmcDisconnect(&MMCD1);
    /* No more sd card */
    fs_ready = FALSE;
}

/* Working area for the state thread */
static WORKING_AREA(waSD, 512);

/*
 * Thread that initializes sd and event listener
 */
static msg_t sdThread(void *arg) {
    (void) arg;
    /* Initializes mmc */
    mmcObjectInit(&MMCD1);
    mmcStart(&MMCD1, &mmccfg);
  
    /* Event listener for insersion/removing */
    struct EventListener el0, el1;
  
    /*
     * Activates the card insertion monitor.
     */
    tmr_init(&MMCD1);
  
    /* Handlers */
    static const evhandler_t evhndl[] = {
      InsertHandler,
      RemoveHandler
    };
  
    current_dir[0] = 0;

    /* Register event */
    chEvtRegister(&inserted_event, &el0, 0);
    chEvtRegister(&removed_event, &el1, 1);
    while (TRUE) {
        chEvtDispatch(evhndl, chEvtWaitOne(ALL_EVENTS));
    }
    return 0;
}

/*
 * Initializes the sd card.
 */
void sdPersoInit() {
    (void)chThdCreateStatic(waSD, sizeof(waSD),
                          NORMALPRIO, sdThread, NULL);
}

/*
 * Command for the console.
 */
void cmdTree(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  uint32_t clusters;
  FATFS *fsp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: tree\r\n");
    return;
  }
  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }
  /* Total size and free size */
  err = f_getfree("/", &clusters, &fsp);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_getfree() failed\r\n");
    return;
  }
  chprintf(chp,
           "FS: %lu free clusters, %lu sectors per cluster, %lu bytes free\r\n",
           clusters, (uint32_t)MMC_FS.csize,
           clusters * (uint32_t)MMC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE);
  fbuff[0] = 0;
  /* Prints the files */
  scan_files(chp, (char *)fbuff);
}

