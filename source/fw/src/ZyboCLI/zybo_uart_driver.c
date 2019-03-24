//////////////////////////////////////////////////////////////////
// These are the functions require to enable a CLI interface
// over UART using the Zybo platform
// Many of these are taken from the Demo source code of FreeRTOS
//////////////////////////////////////////////////////////////////

/* Standard includes. */
#include "string.h"
#include "stdio.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"

/* Demo application includes. */
#include "serial.h"

#include "zybo_uart_driver.h"

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "Welcome to the Zybo Sampler!.\r\nType help to view a list of registered commands.\r\n\r\n>> ";
static const char * const pcEndOfOutputMessage = "\r\n[Press ENTER to execute the previous command again]\r\n>> ";
static const char * const pcNewLine = "\r\n";

/* Used to guard access to the UART in case messages are sent to the UART from
more than one task. */
static SemaphoreHandle_t xTxMutex = NULL;

/* The handle to the UART port, which is not used by all ports. */
static xComPortHandle xPort = 0;

// This task comes from the Zynq CLI demo from the FreeRTOS source code
// -------------
// This is responsible for receiving all the inputs from the UART console and
// stitching it together to form the command that will be passed to the FreeRTOS CLI routines
// -------------
// This task should be called from the scheduler
static void prvUARTCommandConsoleTask( void *pvParameters )
{
    signed char    cRxedChar;                             // Character received 
    static char    cInputString[ cmdMAX_INPUT_SIZE ];     // This array will hold the characters received in a string
    static char    cLastInputString[ cmdMAX_INPUT_SIZE ]; // This will hold the last command in case the user presses ENTER without any input (i.e. repeats the last command)
    uint8_t        ucInputIndex = 0;                      // Idex pointer for cInputString
    char           *pcOutputString;                       // Pointer to the buffer that will be used to store the CLI application output
    BaseType_t     xReturned;                             // This will hold the exit state of the CLI command (pdFALSE == Command is done generating output strings)
    xComPortHandle xPort;                                 // UART Port
    char           *clear_screen;

	( void ) pvParameters; // We are not using the pvParameters right now. Using this to avoid warnings.

    sprintf( clear_screen, "%c[2J\n\r",27 );

	/* Obtain the address of the output buffer.  Note there is no mutual
	exclusion on this buffer as it is assumed only one command console interface
	will be used at any one time. */
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	/* Initialise the UART. */
	xPort = xSerialPortInitMinimal( configCLI_BAUD_RATE, cmdQUEUE_LENGTH );

	/* Send the welcome message. */
	vSerialPutString( xPort, ( signed char * ) clear_screen, ( unsigned short ) strlen( clear_screen ) );
	vSerialPutString( xPort, ( signed char * ) pcWelcomeMessage, ( unsigned short ) strlen( pcWelcomeMessage ) );

	for( ;; )
	{
		/* Wait for the next character.  The while loop is used in case
		INCLUDE_vTaskSuspend is not set to 1 - in which case portMAX_DELAY will
		be a genuine block time rather than an infinite block time. */
		while( xSerialGetChar( xPort, &cRxedChar, portMAX_DELAY ) != pdPASS );

		/* Ensure exclusive access to the UART Tx. */
		if( xSemaphoreTake( xTxMutex, cmdMAX_MUTEX_WAIT ) == pdPASS )
		{
			/* Echo the character back. */
			xSerialPutChar( xPort, cRxedChar, portMAX_DELAY );

			/* Was it the end of the line? */
			if( cRxedChar == '\n' || cRxedChar == '\r' )
			{
				

				/* See if the command is empty, indicating that the last command
				is to be executed again. */
				if( ucInputIndex == 0 )
				{
					/* Copy the last command back into the input string. */
					strcpy( cInputString, cLastInputString );
                    // Printout the command
                    vSerialPutString( xPort, ( signed char * ) cInputString, ( unsigned short ) strlen( cInputString ) );
				} 

                /* Just to space the output from the input. */
				vSerialPutString( xPort, ( signed char * ) pcNewLine, ( unsigned short ) strlen( pcNewLine ) );

				/* Pass the received command to the command interpreter.  The
				command interpreter is called repeatedly until it returns
				pdFALSE	(indicating there is no more output) as it might
				generate more than one string. */
				do
				{

					/* Get the next output string from the command interpreter. */
					xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

					/* Write the generated string to the UART. */
					if ( pcOutputString[ 0 ] != 0x0 ) vSerialPutString( xPort, ( signed char * ) pcOutputString, ( unsigned short ) strlen( pcOutputString ) );

				} while( xReturned != pdFALSE );

				/* All the strings generated by the input command have been
				sent.  Clear the input string ready to receive the next command.
				Remember the command that was just processed first in case it is
				to be processed again. */
				strcpy( cLastInputString, cInputString );
				ucInputIndex = 0;
				memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

				vSerialPutString( xPort, ( signed char * ) pcEndOfOutputMessage, ( unsigned short ) strlen( pcEndOfOutputMessage ) );
			}
			else
			{
				if( cRxedChar == '\r' )
				{
					/* Ignore the character. */
				}
				else if( ( cRxedChar == '\b' ) || ( cRxedChar == cmdASCII_DEL ) )
				{
					/* Backspace was pressed.  Erase the last character in the
					string - if any. */
					if( ucInputIndex > 0 )
					{
						ucInputIndex--;
						cInputString[ ucInputIndex ] = '\0';
					}
				}
				else
				{
					/* A character was entered.  Add it to the string entered so
					far.  When a \n is entered the complete	string will be
					passed to the command interpreter. */
					if( ( cRxedChar >= ' ' ) && ( cRxedChar <= '~' ) )
					{
						if( ucInputIndex < cmdMAX_INPUT_SIZE )
						{
							cInputString[ ucInputIndex ] = cRxedChar;
							ucInputIndex++;
						}
					}
				}
			}

			/* Must ensure to give the mutex back. */
			xSemaphoreGive( xTxMutex );
		}
	}
}

// This task comes from the Zynq CLI demo from the FreeRTOS source code
// -------------
// This function creates the prvUARTCommandConsoleTask
// This is intended to be called in the initialization phases
void vUARTCommandConsoleStart( configSTACK_DEPTH_TYPE usStackSize, UBaseType_t uxPriority )
{
	/* Create the semaphore used to access the UART Tx. */
	xTxMutex = xSemaphoreCreateMutex();
	configASSERT( xTxMutex );

	/* Create that task that handles the console itself. */
	xTaskCreate( 	prvUARTCommandConsoleTask,	/* The task that implements the command console. */
					"CLI",						/* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
					usStackSize,				/* The size of the stack allocated to the task. */
					NULL,						/* The parameter is not used, so NULL is passed. */
					uxPriority,					/* The priority allocated to the task. */
					NULL );						/* A handle is not required, so just pass NULL. */
}
