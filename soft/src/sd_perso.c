#include "sd_perso.h"
#include "ff.h"
#include "string.h"
#include "chprintf.h"
#include "usb_serial.h"

#define POLLING_INTERVAL                10
#define POLLING_DELAY                   10

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
    cat(chp, argv[0]);
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

    FRESULT res;
    res = f_getcwd((TCHAR *) current_dir, sizeof(current_dir));
    if (res) {
        chprintf(chp, "Error while getting the current path");
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
    FRESULT res;
    res = f_getcwd((TCHAR *) current_dir, sizeof(current_dir));
    if (res) {
        chprintf(chp, "Error while getting the current path");
        return;
    }
    chprintf(chp, "Current directory : %s\r\n", current_dir);
}

/*
 * cd command for the shell.
 */
void cmdCd(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
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
        FRESULT res;
        res = f_chdir((TCHAR *) argv[0]);
        if (res) {
            chprintf(chp,"Error while changing the directory\r\n");
            return;
        }
        chprintf(chp, "Current directory : %s\r\n", argv[0]);
    }
}

/*
 * mkdir command for the shell.
 */
void cmdMkdir(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        chprintf(chp, "Usage: mkdir dirname\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    FRESULT res;
    res = f_mkdir(argv[0]);
    if (res) {
        chprintf(chp, "Cannot create that directory\r\n");
    }
}


/*
 * rm command for the shell.
 */
void cmdRm(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        chprintf(chp, "Usage: rm name\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    FRESULT res;
    res = f_unlink(argv[0]);
    if (res) {
        chprintf(chp, "Cannot delete this file\r\n");
    }
}


/*
 * touch command for the shell.
 */
void cmdTouch(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc != 1) {
        chprintf(chp, "Usage: touch name\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    FRESULT res;
    FIL* fil = NULL;
    res = f_open(fil,argv[0],FA_CREATE_NEW);
    if (res == FR_EXIST) {
        chprintf(chp, "The file already exists\r\n");
    } else if (res) {
        chprintf(chp, "Cannot create this file\r\n");
    }
    if (fil != NULL)
        f_close(fil);
}

/*
 * mv command for the shell.
 */
void cmdMv(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if (argc != 2) {
        chprintf(chp, "Usage: rm old_name new_name\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }
    FRESULT res;
    res = f_rename(argv[0],argv[1]);
    if (res) {
        chprintf(chp, "Cannot move this file\r\n");
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

    /* Initialize SPI pins */
    palSetPadMode(GPIOD, 10, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOD,10);
    palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5) | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5) | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5) | PAL_STM32_PUDR_PULLUP);

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

/* test mkdir and rm functions */
FRESULT testMkdir(BaseSequentialStream *chp, char * dirname) {
    FRESULT err;
    char * args[1];

    chprintf(chp, "Create a directory\r\n");
    /* Create a directory */
    args[0] = dirname;
    cmdMkdir(chp, 1, args);
    chprintf(chp, "End of the directory creation\r\n");

    /* Checks if the file exists */
    FILINFO* fno = NULL;
    err = f_stat(dirname, fno);

    if (err == FR_NO_FILE) {
        chprintf(chp, "The directory was not created\r\n");
    } else if (err) {
        chprintf(chp, "An error occured after directory creation\r\n");
    }
    return err;
}

/* test rm function */
FRESULT testRm(BaseSequentialStream *chp, char * name) {
    FRESULT err;
    char * args[1];

    chprintf(chp, "Remove the directory\r\n");
    /* Remove the directory created */
    args[0] = name;
    cmdRm(chp, 1, args);
    chprintf(chp, "End of the directory removal\r\n");

    /* Checks if the file was removed */
    FILINFO* fno = NULL;
    err = f_stat(name, fno);

    if (err == FR_NO_FILE) {
        chprintf(chp, "The directory was removed\r\n");
        return FR_OK;
    } else if (err) {
        chprintf(chp, "An error occured after directory removal\r\n");
        return err;
    }

    return 1;
}

FRESULT testTouch(BaseSequentialStream *chp, char * filename){
    FRESULT err;
    char * args[1];

    chprintf(chp, "Create a file\r\n");
    /* Create a directory */
    args[0] = filename;
    cmdTouch(chp, 1, args);
    chprintf(chp, "End of the file creation\r\n");

    /* Checks if the file exists */
    FILINFO* fno = NULL;
    err = f_stat(filename, fno);

    if (err == FR_NO_FILE) {
        chprintf(chp, "The file was not created\r\n");
    } else if (err) {
        chprintf(chp, "An error occured after file creation\r\n");
    }
    return err;
}

FRESULT testMv(BaseSequentialStream *chp, char * from, char * to) {
    FRESULT err;
    char * args[2];

    args[0] = from;
    args[1] = to;
    chprintf(chp, "Begin to move the file\r\n");
    cmdMv(chp,2, args);
    chprintf(chp, "End of the moving action\r\n");

    /* Checks if the file exists */
    FILINFO* fno = NULL;
    err = f_stat(to, fno);

    if (err == FR_NO_FILE) {
        chprintf(chp, "The file was not moved\r\n");
    } else if (err) {
        chprintf(chp, "An error occured after the moving action\r\n");
    }
    return err;
}

/* Test Write/read */
FRESULT testWR(BaseSequentialStream *chp){
    /* File object */
    FIL fil;
    FRESULT res;

    chprintf(chp, "Open a file in write mode\r\n");
    res = f_open(&fil, "testwr", FA_WRITE | FA_CREATE_NEW);
    if(res) {
        chprintf(chp,"A problem occured while opening the file\r\n");
        return res;
    }
    chprintf(chp, "File opened\r\n");
    chprintf(chp, "Write data\r\n");
    int written = f_printf(&fil, (const TCHAR*) "This is a test !\n");
    if (written != 17) {
        chprintf(chp, "The characters were not all written\r\n");
        testRm(chp, "testwr");
        return 1;
    }
    chprintf(chp, "Data written\r\n");
    chprintf(chp, "Close file\r\n");
    res = f_close(&fil);
    if (res) {
        chprintf(chp, "An error occured while opening the file\r\n");
        testRm(chp, "testwr");
        return res;
    }
    chprintf(chp, "Open a file in read mode\r\n");
    res = f_open(&fil, "testwr", FA_READ);
    if(res) {
        chprintf(chp,"A problem occured while opening the file\r\n");
        return res;
    }
    chprintf(chp, "File opened\r\n");
    char buff[20];
    chprintf(chp, "Read data\r\n");
    char * buffres = f_gets(buff, sizeof(buff), &fil);
    if (buff != buffres){
        chprintf(chp, "A problem occured while reading data\r\n");
        testRm(chp, "testwr");
        return 1;
    }
    if (strcmp(buff, "This is a test !\n") != 0){
        chprintf(chp, "The datas read are not the same than the one written\r\n");
        testRm(chp, "testwr");
        return 1;
    }
    chprintf(chp, "The reading is OK\r\n");
    chprintf(chp, "Close file\r\n");
    res = f_close(&fil);
    if (res) {
        chprintf(chp, "An error occured while opening the file\r\n");
        testRm(chp, "testwr");
        return res;
    }
    return testRm(chp, "testwr");
}

/* Test the functionalities of the sd card */
void testSd(BaseSequentialStream *chp, int argc, char * argv[]){
    (void)argv;
    if (argc > 0) {
        chprintf(chp, "Usage: tree\r\n");
        return;
    }
    if (!fs_ready) {
        chprintf(chp, "File System not mounted\r\n");
        return;
    }

    /* mkdir */
    if(testMkdir(chp,"abc123")) goto ERROR;

    /* mv dir */
    if(testMv(chp, "abc123", "123abc")) goto ERROR;

    /* rm dir */
    if(testRm(chp, "123abc")) goto ERROR;

    /* touch */
    if(testTouch(chp, "123abc")) goto ERROR;

    /* mv file */
    if(testMv(chp, "123abc", "abc123")) goto ERROR;

    /* rm file */
    if(testRm(chp, "abc123")) goto ERROR;

    /* test write and read in a file */
    if(testWR(chp)) goto ERROR;

    chprintf(chp, "The SD test SUCCEEDED\r\n");

    return;

ERROR :
    chprintf(chp, "The SD test FAILED\r\n");
}

FRESULT writeFile(char * filename, char * buf, UINT length){
    if (!fs_ready) {
        return 1;
    }

    /* File object */
    FIL fil;
    FRESULT res;

    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
    if(res) {
        return res;
    }

    UINT written = 0;
    f_write(&fil, buf, length, &written);
    if (written != length) {
        f_unlink(filename);
        return 1;
    }

    res = f_close(&fil);
    if (res) {
        f_unlink(filename);
        return res;
    }
    return res;
}
