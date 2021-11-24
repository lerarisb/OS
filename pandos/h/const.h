#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/
#define MAXPROC           20

/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define RAMBASESIZE		0x10000004
#define TODLOADDR		  0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024

#define RAMTOP  (RAMBASEADDR + RAMBASESIZE)

#define GETEXECCODE  0x0000007C


/*exception constants */
#define INTERRUPT   0
#define TLBCAUSE    3
#define SYSCALLHANDLE     8


/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'

#define NULL 			    ((void *)0xFFFFFFFF)

/* device interrupts */
#define DISKINT			  3
#define FLASHINT 		  4
#define NETINT 		  5
#define PRNTINT 		  6
#define TERMINT			  7

#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */	
#define DEVREGSIZE	  16 		/* device register size in bytes */
#define SEM4DEV           (DEVINTNUM * DEVPERINT + 9)

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			    2
#define DATA1			    3

/* device register field number for terminal devices */
#define RECVSTATUS  	0
#define RECVCOMMAND 	1
#define TRANSTATUS  	2
#define TRANCOMMAND 	3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			    1
#define BUSY			    3

#define PASSUPVECTOR    0x0FFFF900
#define KERNELSTACK     0x20001000

/* device common COMMAND codes */
#define RESET			    0
#define ACK				    1
#define ON                  1
#define OFF                 0

/* Memory related constants */
#define KSEG0           0x00000000
#define KSEG1           0x20000000
#define KSEG2           0x40000000
#define KUSEG           0x80000000
#define RAMSTART        0x20000000
#define BIOSDATAPAGE    0x0FFFF000
#define	PASSUPVECTOR	  0x0FFFF900

/* Exceptions related constants */
#define	PGFAULTEXCEPT	  0
#define GENERALEXCEPT	  1

/*Constants for bit management */
#define ALLOFF      0x00000000
#define TEON        0x08000000
#define IEON        0x00000010
#define MASKON      0x0000FF00


#define CAUSESHIFT  2


/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))


#define DEV0 0x00000001
#define DEV1 0x00000002
#define DEV2 0x00000004
#define DEV3 0x00000008
#define DEV4 0x00000010
#define DEV5 0x00000020
#define DEV6 0x00000040
#define DEV7 0x00000080

/*syscall constants */

#define CREATETHREAD 1
#define TERMINATETHREAD 2
#define PASSERREN   3
#define VERHOGEN 4
#define WAITIO  5
#define GETCPUTIME  6
#define WAITCLOCK   7
#define GETSUPPORTPTR   8


#define PLTINTERRUPT 0x00000200 /* PLT interrupt */
#define TIMERINT 0x00000400 /* clock 2 interrupt */
#define DISKINTERRUPT 0x00000800  /* disk interrupt */
#define FLASHINTERRUPT 0x00001000  /* flash interrupt */
#define NETWORKINTERRUPT 0x00002000   /* network interrupt */
#define PRINTERINTERRUPT 0x00004000  /* printer interrupt */
#define TERMINTERRUPT 0x00008000  /* terminal interrupt */

#define PSEUDOCLOCKTIME 100000
#define MAXTIME 0xFFFFFFFF
#define QUANTUM 5000
#define IOCLOCK 100000

#define IMON    0x0000FF00      /* Interrupt Masked */
#define IECON   0x00000001      /* Turn interrupts current ON */
#define TEBITON 0x08000000      /*Timer enabled bit ON */

#define USERON  0x00000008  /* user mode on */

#endif

