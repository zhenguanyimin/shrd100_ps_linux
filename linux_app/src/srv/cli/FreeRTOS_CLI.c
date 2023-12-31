
// #include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
/* Utils includes. */
#include "FreeRTOS_CLI.h"
#include "eap_os_adapter.h"

/* If the application writer needs to place the buffer used by the CLI at a
fixed address then set configAPPLICATION_PROVIDES_cOutputBuffer to 1 in
FreeRTOSConfig.h, then declare an array with the following name and size in 
one of the application files:
	char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
*/
#ifndef configAPPLICATION_PROVIDES_cOutputBuffer
	#define configAPPLICATION_PROVIDES_cOutputBuffer 0
#endif

typedef struct cli_mutex
{
	bool				bFlagInit;
	void* 	mutex;
}cli_mutex_t;

static cli_mutex_t sCli = { 0 };

/*
 * The callback function that is executed when "help" is entered.  This is the
 * only default command that is always present.
 */
static BaseType_t prvHelpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Return the number of parameters that follow the command name.
 */
static int8_t prvGetNumberOfParameters( const char *pcCommandString );

/* The definition of the "help" command.  This command is always at the front
of the list of registered commands. */
static const CLI_Command_Definition_t xHelpCommand =
{
	"help",
	"\r\n help:\r\n Lists all the registered commands\r\n\r\n",
	prvHelpCommand,
	0
};

/* The definition of the list of commands.  Commands that are registered are
added to this list. */
static CLI_Definition_List_Item_t xRegisteredCommands =
{
	&xHelpCommand,	/* The first command in the list is always the help command, defined in this file. */
	NULL			/* The next pointer is initialised to NULL, as there are no other registered commands yet. */
};

/* A buffer into which command outputs can be written is declared here, rather
than in the command console implementation, to allow multiple command consoles
to share the same buffer.  For example, an application may allow access to the
command interpreter by UART and by Ethernet.  Sharing a buffer is done purely
to save RAM.  Note, however, that the command console itself is not re-entrant,
so only one command interpreter interface can be used at any one time.  For that
reason, no attempt at providing mutual exclusion to the cOutputBuffer array is
attempted.

configAPPLICATION_PROVIDES_cOutputBuffer is provided to allow the application
writer to provide their own cOutputBuffer declaration in cases where the
buffer needs to be placed at a fixed address (rather than by the linker). */
#if( configAPPLICATION_PROVIDES_cOutputBuffer == 0 )
	static char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
#else
	extern char cOutputBuffer[ configCOMMAND_INT_MAX_OUTPUT_SIZE ];
#endif


/*-----------------------------------------------------------*/
static CLI_Definition_List_Item_t *CliGetProcessCommand(const char * const pcCommandInput, int8_t numParameters)
{
    CLI_Definition_List_Item_t *pxCommand;
    const char *pcRegisteredCommandString;
    size_t xCommandStringLength;

	for( pxCommand = &xRegisteredCommands; pxCommand != NULL; pxCommand = pxCommand->pxNext )
	{
		pcRegisteredCommandString = pxCommand->pxCommandLineDefinition->pcCommand;
		xCommandStringLength = strlen( pcRegisteredCommandString );

		if( strncmp( pcCommandInput, pcRegisteredCommandString, xCommandStringLength ) == 0 )
		{
			if( ( pcCommandInput[ xCommandStringLength ] == ' ' ) || ( pcCommandInput[ xCommandStringLength ] == 0x00 ) )
			{
				if ( numParameters == pxCommand->pxCommandLineDefinition->cExpectedNumberOfParameters )
				{
                    return pxCommand;
				}
			}
            return NULL;
		}
	}
    return NULL;
}

BaseType_t FreeRTOS_CLIRegisterCommand( const CLI_Command_Definition_t * const pxCommandToRegister )
{
static CLI_Definition_List_Item_t *pxLastCommandInList = &xRegisteredCommands;
CLI_Definition_List_Item_t *pxNewListItem;
BaseType_t xReturn = pdFAIL;

	/* Check the parameter is not NULL. */
	// configASSERT( pxCommandToRegister );
	assert(pxCommandToRegister);

	/* Create a new list item that will reference the command being registered. */
	pxNewListItem = ( CLI_Definition_List_Item_t * ) malloc( sizeof( CLI_Definition_List_Item_t ) );
	// pxNewListItem = ( CLI_Definition_List_Item_t * ) pvPortMalloc( sizeof( CLI_Definition_List_Item_t ) );
	// configASSERT( pxNewListItem );
	assert(pxNewListItem);
	if (!sCli.bFlagInit)
	{
		int32_t eRet = 0;
		sCli.mutex = eap_create_mutex();
	}

	if( pxNewListItem != NULL )
	{
		eap_mutex_lock(sCli.mutex);

		if (NULL == CliGetProcessCommand(pxCommandToRegister->pcCommand, pxCommandToRegister->cExpectedNumberOfParameters))
        {
            /* Reference the command being registered from the newly created
			list item. */
			pxNewListItem->pxCommandLineDefinition = pxCommandToRegister;

			/* The new list item will get added to the end of the list, so
			pxNext has nowhere to point. */
			pxNewListItem->pxNext = NULL;

			/* Add the newly created list item to the end of the already existing
			list. */
			pxLastCommandInList->pxNext = pxNewListItem;

			/* Set the end of list marker to the new list item. */
			pxLastCommandInList = pxNewListItem;
		}
		eap_mutex_unlock(sCli.mutex);

		xReturn = pdPASS;
	}

	return xReturn;
}

BaseType_t FreeRTOS_CLIProcessCommand( const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen  )
{
    static const CLI_Definition_List_Item_t *pxCommand = NULL;
    BaseType_t xReturn = pdTRUE;
    const char *pcRegisteredCommandString;
    size_t xCommandStringLength;
    int8_t numParams = prvGetNumberOfParameters(pcCommandInput);


	eap_mutex_lock(sCli.mutex);
	/* Note:  This function is not re-entrant.  It must not be called from more
	thank one task. */
    if (pxCommand == NULL)
    {
        pxCommand = CliGetProcessCommand(pcCommandInput, numParams);
    }
	if (pxCommand != NULL)
	{
		/* Call the callback function that is registered to this command. */
		xReturn = pxCommand->pxCommandLineDefinition->pxCommandInterpreter( pcWriteBuffer, xWriteBufferLen, pcCommandInput );

		/* If xReturn is pdFALSE, then no further strings will be returned
		after this one, and	pxCommand can be reset to NULL ready to search
		for the next entered command. */
		if( xReturn == pdFALSE )
		{
			pxCommand = NULL;
		}
	}
	else
	{
		/* pxCommand was NULL, the command was not found. */
		strncpy( pcWriteBuffer, "Command not recognised.  Enter 'help' to view a list of available commands.\r\n\r\n", xWriteBufferLen );
		xReturn = pdFALSE;
	}
	
	eap_mutex_unlock(sCli.mutex);

	return xReturn;
}
/*-----------------------------------------------------------*/

char *FreeRTOS_CLIGetOutputBuffer( void )
{
	return cOutputBuffer;
}
/*-----------------------------------------------------------*/

const char *FreeRTOS_CLIGetParameter( const char *pcCommandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength )
{
UBaseType_t uxParametersFound = 0;
const char *pcReturn = NULL;

	*pxParameterStringLength = 0;

	while( uxParametersFound < uxWantedParameter )
	{
		/* Index the character pointer past the current word.  If this is the start
		of the command string then the first word is the command itself. */
		while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) != ' ' ) )
		{
			pcCommandString++;
		}

		/* Find the start of the next string. */
		while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) == ' ' ) )
		{
			pcCommandString++;
		}

		/* Was a string found? */
		if( *pcCommandString != 0x00 )
		{
			/* Is this the start of the required parameter? */
			uxParametersFound++;

			if( uxParametersFound == uxWantedParameter )
			{
				/* How long is the parameter? */
				pcReturn = pcCommandString;
				while( ( ( *pcCommandString ) != 0x00 ) && ( ( *pcCommandString ) != ' ' ) )
				{
					( *pxParameterStringLength )++;
					pcCommandString++;
				}

				if( *pxParameterStringLength == 0 )
				{
					pcReturn = NULL;
				}

				break;
			}
		}
		else
		{
			break;
		}
	}

	return pcReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvHelpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
static const CLI_Definition_List_Item_t * pxCommand = NULL;
BaseType_t xReturn;

	( void ) pcCommandString;

	if( pxCommand == NULL )
	{
		/* Reset the pxCommand pointer back to the start of the list. */
		pxCommand = &xRegisteredCommands;
	}

	/* Return the next command help string, before moving the pointer on to
	the next command in the list. */
	strncpy( pcWriteBuffer, pxCommand->pxCommandLineDefinition->pcHelpString, xWriteBufferLen );
	pxCommand = pxCommand->pxNext;

	if( pxCommand == NULL )
	{
		/* There are no more commands in the list, so there will be no more
		strings to return after this one and pdFALSE should be returned. */
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

static int8_t prvGetNumberOfParameters( const char *pcCommandString )
{
int8_t cParameters = 0;
BaseType_t xLastCharacterWasSpace = pdFALSE;

	/* Count the number of space delimited words in pcCommandString. */
	while( *pcCommandString != 0x00 )
	{
		if( ( *pcCommandString ) == ' ' )
		{
			if( xLastCharacterWasSpace != pdTRUE )
			{
				cParameters++;
				xLastCharacterWasSpace = pdTRUE;
			}
		}
		else
		{
			xLastCharacterWasSpace = pdFALSE;
		}

		pcCommandString++;
	}

	/* If the command string ended with spaces, then there will have been too
	many parameters counted. */
	if( xLastCharacterWasSpace == pdTRUE )
	{
		cParameters--;
	}

	/* The value returned is one less than the number of space delimited words,
	as the first word should be the command itself. */
	return cParameters;
}

const CLI_Definition_List_Item_t *GetCLIRegisteredCommands()
{
    return &xRegisteredCommands;
}