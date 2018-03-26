/*
  This Software is proprietary to Unigraphics Solutions Inc.
  Copyright (c) 1999 Unigraphics Solutions Inc.
  This program is an unpublished work fully protected by the United States
  copyright laws and is considered a trade secret belonging to the copyright
  holder. All rights reserved.
  Restricted Rights Legend: Use, Duplication or Disclosure by the Government
  is Subject to Restrictions as Set Forth in Paragraph (c)(1)(ii) of the
  Rights in Technical Data and Computer Software Clause at DFARS 252.227-7013.

*/

/*	This file contains the frustrum files and a number of helper functions relating
	to externally storing and accessing data within the application. This could take 
	the form of a controlled directory structure on the host computer, or some kind of
	database.
	There are a number of ways that this can be implemented, however this 
	implementation of the frustrum is based on the use of files provided by the NT
	operating system

	Note: The graphics frustrum functions are defined and registered in CExampleAppDoc
*/


/*=============================================================================
                 #INCLUDES
=============================================================================*/

#include "../stdafx.h"
#include "frustrum_ifails.h"
#include "frustrum_tokens.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

//#include <windows.h>

/*=============================================================================
                 #DEFINES
=============================================================================*/

/* extra useful ifails */

#define  FR_not_started       FR_unspecified
#define  FR_internal_error    FR_unspecified


/* other useful definitions */

#define null_strid          (-1)
#define max_namelen         255         /* maximum length of a full pathname */
#define max_header_line (max_namelen+32) /* for long FILE=name in the header */
#define max_open_files      32

#define read_access         1
#define write_access        2
#define read_write_access   3

#define end_of_string_c     '\0'
#define end_of_string_s     "\0"

#define new_line_c          '\n'
#define new_line_s          "\n"

#define semi_colon_c        ';'
#define semi_colon_s        ";"

#define dir_separator_c     '\\'
#define dir_separator_s     "\\"


/*=============================================================================
                 STRUCTS
=============================================================================*/


/* one structure per open file containing info such as filename and
   the C stream id. the structures are chained together, accessed via
   the "open_files" variable
*/

typedef struct file_s *file_p;

typedef struct file_s
{
  file_p next;
  file_p prev;
  int    strid;
  int    guise;
  int    format;
  int    access;
  char   name[max_namelen+1];
  char   key[max_namelen+1];
  FILE  *stream;
} file_t;

static file_p open_files = NULL;

/* file stream identifiers and count of open files */
static int stream_id[max_open_files];
static int file_count = 0;

/*=============================================================================
                 GLOBAL VARIABLES
=============================================================================*/

/* the following are for writing and checking file headers */

static char g_preamble_1[ max_header_line ] = end_of_string_s;
static char g_preamble_2[ max_header_line ] = end_of_string_s;
static char g_prefix_1[ max_header_line ] = "**PART1;\n";
static char g_prefix_2[ max_header_line ] = "**PART2;\n";
static char g_prefix_3[ max_header_line ] = "**PART3;\n";
static char g_trailer_start[ max_header_line ] = "**END_OF_HEADER";
static char g_trailer[ max_header_line ] = end_of_string_s;
static char g_unknown_value[] = "unknown";

/* machine specific: fopen file open modes. On NT platforms use binary */
/* mode to suppress the writing of carriage returns before line feeds  */

static char g_fopen_mode_read_text[]   = "r";
static char g_fopen_mode_read_binary[] = "rb";
static char g_fopen_mode_write[]       = "wb";
static char g_fopen_mode_append[]      = "wb+";

/* this buffer used for input-output of file headers and text files */
static char *input_output_buffer = NULL;
static int   input_output_buflen = 0;

/*=============================================================================
                 UTILITY FUNCTIONS
=============================================================================*/

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: format_string

Description:

  Returns a pointer to a lowercase string which declares the file format
  (binary or text). This is for writing into file headers.


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static char* format_string( int format )
    {
    static char ffbnry[] = "binary";
    static char fftext[] = "text";
    switch( format )
      {
      case FFBNRY:
        return ffbnry;
      case FFTEXT:
        return fftext;
      }
    return g_unknown_value;
    }


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: guise_string

Description:

  Returns a pointer to a lowercase string which declares the file guise
  (that is rollback, snapshot, journal, transmit, schema, licence).
  Romulus files do not have headers; guise FFCXMO is  not valid.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static char* guise_string( int guise )
    {
    static char ffcrol[] = "rollback";
    static char ffcsnp[] = "snapshot";
    static char ffcjnl[] = "journal";
    static char ffcxmt[] = "transmit";
    static char ffcxmo[] = "old_transmit";
    static char ffcsch[] = "schema";
    static char ffclnc[] = "licence";
    static char ffcxmp[] = "transmit_partition";
    static char ffcxmd[] = "transmit_deltas";
	static char ffcdbg[] = "debug_report";

    switch ( guise )
      {
      case FFCROL: return ffcrol;
      case FFCSNP: return ffcsnp;
      case FFCJNL: return ffcjnl;
      case FFCXMT: return ffcxmt;
      case FFCSCH: return ffcsch;
      case FFCLNC: return ffclnc;
      case FFCXMP: return ffcxmp;
      case FFCXMD: return ffcxmd;
	  case FFCDBG: return ffcdbg;
      }
    return g_unknown_value;
    }

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: filetype_guise_string

Description:

  Returns a pointer to a filetype string for the specified guise.
  Used in the construction of filenames in FFOPRD, FFOPWR etc.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static char* filetype_guise_string( int guise )
	{
	// Here we are assuming 3 character extensions
    static char ffcsnp[] = ".N";
    static char ffcjnl[] = ".J";
    static char ffcxmt[] = ".X";
    static char ffcsch[] = ".S";
    static char ffclnc[] = ".L";
    static char ffcxmo[] = ".XMT";
    static char ffcxmp[] = ".P";
    static char ffcxmd[] = ".D";
	static char ffcdbg[] = ".XML";

    switch( guise )
        {
        case FFCSNP:
            return ffcsnp;
        case FFCJNL:
            return ffcjnl;
        case FFCXMT:
            return ffcxmt;
        case FFCSCH:
            return ffcsch;
        case FFCLNC:
            return ffclnc;
        case FFCXMO:
            return ffcxmo;
        case FFCXMP:
            return ffcxmp;
        case FFCXMD:
            return ffcxmd;
		case FFCDBG:
			return ffcdbg;
        }

    return end_of_string_s;
    }


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: filetype_format_string

Description:

  Returns a pointer to a file format (text or binary) filetype string
  for the specified format.
  Used in the construction of filenames in FFOPRD, FFOPWR etc.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static char* filetype_format_string( int format )
	// Here we are assuming 3 character extensions
    {
    static char ffbnry[] = "_B";
    static char fftext[] = "_T";

    switch ( format )
        {
        case FFBNRY:
            return ffbnry;
        case FFTEXT:
            return fftext;
        }

    return end_of_string_s;
    }

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: delete_file

Description:

  Deletes the named file.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void delete_file( char* name, int* ifail )
    {
    if(remove( name ) != 0)
        {
        *ifail = FR_close_fail;
        }
    else
        *ifail = FR_no_errors;
    }

/*=============================================================================
                    FILE HANDLING FUNCTIONS
=============================================================================*/


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: new_open_file

Description:

  Allocates new structure, adds the file information and
  adds it to the list of open-file structures.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void new_open_file( FILE* stream, int guise, int format, int access,
			   char* filename, char* keyname, file_p* file_ptr,
			   int* ifail )
    {
    file_p ptr;
    file_p temp;
    int i;

    /* allocate and add file structure into list of open files */
    ptr = (file_p) malloc( sizeof( file_t ));
    if (ptr == NULL)
      {
        fclose( stream );
        {
        *ifail = FR_open_fail;
        return;
        }
      }

    if (open_files == NULL)
        open_files = ptr;
    else
      {
        for ( temp = open_files; temp->next != NULL; temp = temp->next )
            /* skip */;
        temp->next = ptr;
      }

    /* initialise file structure */
    ptr->next = NULL;
    if (open_files == ptr)
        ptr->prev = NULL;
    else
        ptr->prev = temp;
    for ( i = 0; stream_id[i] != 0; i++ )
        /* skip */;

    stream_id[i] = i + 1;
    ptr->strid = i + 1;
    ptr->guise = guise;
    ptr->format = format;
    ptr->access = access;
    ptr->stream = stream;

    strcpy( ptr->name, filename );
    strcpy( ptr->key, keyname );

    file_count++;

    *file_ptr = ptr;
    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: write_to_file

Description:

  Writes given buffer to open file as either ascii or binary.
  Uses 'fputs' for ascii and 'fwrite' for binary.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void write_to_file( file_p file_ptr, const char* buffer, int header,
			   int buffer_len, int* ifail)
    {
    /* in this example frustrum, the headers of binary and text files are */
    /* output with fputs() and other binary data are output with fwrite() */

    if (header || file_ptr->format == FFTEXT)
        {
        if ( buffer_len == 1 )
            {
            if (fputc( buffer[0], file_ptr->stream ) == EOF)
              {
              *ifail = FR_write_fail;
              }
            else
              {
              *ifail = FR_no_errors;
              }
            }
        else
            {
            int required;
            int count;
            /* check whether the global input-output buffer is long enough */
            required = (buffer_len + 1) * sizeof( char );

            if ( input_output_buflen < required )
                {
                if ( input_output_buffer != NULL )
                    free(input_output_buffer);

                input_output_buflen = 0;
                input_output_buffer = (char *) malloc( required );

                if ( input_output_buffer == NULL )
                    {
                    *ifail = FR_unspecified;
                    return;
                    }
                else
                    input_output_buflen = required;
                }


            /* copy the buffer and add a null-terminating character */

            for( count = 0; count < buffer_len; count ++ )
              {
                input_output_buffer[ count ] = buffer[ count ];
              }
            input_output_buffer[buffer_len] = end_of_string_c;

            /* the string will already contain any necessary formatting characters
              (added by the header routines or Parasolid); fputs does not add any
            */

            if (fputs( input_output_buffer, file_ptr->stream ) == EOF)
              {
                *ifail = FR_write_fail;
              }
            else
              {
              *ifail = FR_no_errors;
              }
            }
        }
    else
        {
        /* write to binary file */
        int written;
        written = fwrite( buffer, (unsigned) (sizeof(char)),
                          buffer_len, file_ptr->stream );
        if ( written != buffer_len)
          {
            *ifail = FR_write_fail;
          }
        else
          {
            *ifail = FR_no_errors;
          }
        }
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: read_from_file

Description:

  Read the required amount of data from open file.
  Uses 'fgets' for ascii and 'fread' for binary.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void read_from_file( file_p file_ptr, char* buffer, int header,
			    int max_buffer_len, int* buffer_len, int* ifail)
    {
    /* in this example frustrum, the headers of binary and text files are */
    /* input with fgets()  and  other binary data are input with fread()  */

    if (header || file_ptr->format == FFTEXT)
        {
        if ( max_buffer_len == 1 )
            {
            int value = fgetc(file_ptr->stream);
            if ( value == EOF )
                {
                *ifail = (feof(file_ptr->stream)?FR_end_of_file:FR_read_fail);
                return;
                }
            else
                {
                buffer[0] = value;
                *buffer_len = 1;
                }
            }
        else
            {
            int required;

            /* check whether current global input-output buffer long enough */
            required = (max_buffer_len + 1) * sizeof( char );

            if ( input_output_buflen < required )
                {
                if ( input_output_buffer != NULL )
                    free(input_output_buffer);

                input_output_buflen = 0;
                input_output_buffer = (char *) malloc( required );

                if ( input_output_buffer == NULL )
                    {
                    *ifail = FR_unspecified;
                    return;
                    }
                else
                    input_output_buflen = required;
                }

            /* note that the second argument to fgets is the maximim number */
            /* of characters which can ever be written (including the null) */
            /* which is why the second argument to fgets = max_buffer_len+1 */
            if (fgets(input_output_buffer, max_buffer_len+1,
                      file_ptr->stream) == NULL)
                {
                *ifail = (feof(file_ptr->stream)?FR_end_of_file:FR_read_fail);

                return;
                }

            /* copy input buffer back to calling function without terminator */
            *buffer_len = strlen(input_output_buffer);
            strncpy(buffer, input_output_buffer, *buffer_len);
            }
        }
    else
        {
        int chars = fread( buffer, (unsigned) (sizeof( char )),
                                max_buffer_len, file_ptr->stream );
        if (chars == 0)
          {
          *ifail = ( feof(file_ptr->stream) ? FR_end_of_file : FR_read_fail );

          return;
          }

        *buffer_len = chars;
        }

    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: check_valid_filename

Description:

  Checks that filename is valid, that no spurious characters are there

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void check_valid_filename( char* filename, int* ifail )
    {
    int len;

    /* check that the string is not too long */
    len = strlen( filename );
    if ( len > max_namelen )
      {
        *ifail = FR_bad_name;
        return;
      }

    *ifail = FR_no_errors;
    }

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: extend_schema_filename

Description:

  Add a path variable detailing the location of the schema files. In
  this case the path is hardcoded but could be stored and read from
  the registry. Prepend the path (and a directory separator) to the
  supplied pathname.
  If the resulting pathname would exceed max_namelen characters, the
  supplied pathname is not modified.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void extend_schema_filename( char* filename )
{
    char extended_filename[max_namelen + 1];
  
    char *p_schema_prefix = "Schemas";
    if ( p_schema_prefix != NULL
    &&  strlen(p_schema_prefix) + 1 + strlen(filename) < max_namelen )
    {
        strcpy( extended_filename, p_schema_prefix );
        strcat( extended_filename, dir_separator_s );
        strcat( extended_filename, filename );
        strcpy( filename, extended_filename );
    }
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: write_xml_header

Description:

  Writes standard xml header to file. At the moment this is only used 
  for debug report output (PK_DEBUG_report_start). This 
  function should output the xml header <?xml version="1.0" ?>

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void write_xml_header( file_p file_ptr, const char* pd2hdr, int pd2len,
			  int* ifail)
{
	char buffer[max_header_line];
	
	// <?xml version
	strcpy( buffer, "<?xml version=\"1.0\" ?>" );
	write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
	if ( *ifail != FR_no_errors ) return;
  
    *ifail = FR_no_errors;
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: write_header

Description:

  Writes standard header to file. Most keyword values are written as
  "unknown". This must be changed straight away to produce meaningful
  text - in particular the frustrum name, application name, date and
  type of machine.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void write_header( file_p file_ptr, const char* pd2hdr, int pd2len,
			  int* ifail)
    {
    char buffer[max_header_line];
    write_to_file( file_ptr, g_preamble_1, 1, strlen(g_preamble_1), ifail );
    if ( *ifail != FR_no_errors ) return;

    write_to_file( file_ptr, g_preamble_2, 1, strlen(g_preamble_2), ifail );
    if ( *ifail != FR_no_errors ) return;

    write_to_file( file_ptr, g_prefix_1, 1, strlen(g_prefix_1), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - the frustrum should write the machine name */
    strcpy( buffer, "MC=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - the frustrum should write the machine model number */
    strcpy( buffer, "MC_MODEL=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - the frustrum should write the machine identifier */
    strcpy( buffer, "MC_ID=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - the frustrum should write the operating system name */
    strcpy( buffer, "OS=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - the frustrum should write the operating system versn */
    strcpy( buffer, "OS_RELEASE=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - this should be replaced by your company name */
    strcpy( buffer, "FRU=sdl_parasolid_customer_support;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - this should be replaced by your product's name */
    strcpy( buffer, "APPL=Example Application;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - this should be replaced by your company's location */
    strcpy( buffer, "SITE=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - this should be replaced by runtime user's login id */
    strcpy( buffer, "USER=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    strcpy( buffer, "FORMAT=" );
    strcat( buffer, format_string( file_ptr->format ) );
    strcat( buffer, ";\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    strcpy( buffer, "GUISE=" );
    strcat( buffer, guise_string( file_ptr->guise ) );
    strcat( buffer, ";\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    strcpy( buffer, "KEY=" );
    strcat( buffer, file_ptr->key );
    strcat( buffer, ";\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    strcpy( buffer, "FILE=" );
    strcat( buffer, file_ptr->name );
    strcat( buffer, ";\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    /* machine specific - this should be replaced by the runtime date */
    strcpy( buffer, "DATE=unknown;\n" );
    write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
    if ( *ifail != FR_no_errors ) return;

    write_to_file( file_ptr, g_prefix_2, 1, strlen(g_prefix_2), ifail );
    if ( *ifail != FR_no_errors ) return;

    {
    int pd2_count, buffer_count;

    buffer_count = 0;
    for (pd2_count = 0; pd2_count < pd2len; pd2_count++ )
      {
      char c = buffer[buffer_count] = pd2hdr[pd2_count];
      if ( c == ';' )
        {
        buffer[ buffer_count +1 ] = new_line_c;
        buffer[ buffer_count +2 ] = end_of_string_c;
        write_to_file( file_ptr, buffer, 1, strlen( buffer ), ifail );
        if ( *ifail != FR_no_errors ) return;
        buffer_count = 0;
        }
      else
        {
        buffer_count++;
        }
      }
    }
    write_to_file( file_ptr, g_prefix_3, 1, strlen(g_prefix_3), ifail );
    if ( *ifail != FR_no_errors ) return;

    write_to_file( file_ptr, g_trailer, 1, strlen(g_trailer), ifail );
    if ( *ifail != FR_no_errors ) return;

    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: setup_header

Description:

  Initialise the global variables storing the text for the stardard
  headers written to files.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void setup_header( void )
    {

    strcpy( g_preamble_1,
              "**" );                           /* two asterisks */
    strcat( g_preamble_1,
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );   /* upper case letters */
    strcat( g_preamble_1,
              "abcdefghijklmnopqrstuvwxyz" );   /* lower case letters */
    strcat( g_preamble_1,
              "**************************" );   /* twenty six asterisks */
    strcat( g_preamble_1, new_line_s );


    strcpy( g_preamble_2,
              "**" );                           /* two asterisks */
    strcat( g_preamble_2,
              "PARASOLID" );                    /* PARASOLID (upper case) */
    strcat( g_preamble_2,
              " !" );                           /* space and exclamation */
    strcat( g_preamble_2,
              "\"" );                           /* a double quote char */
    strcat( g_preamble_2,
              "#$%&'()*+,-./:;<=>?@[" );        /* some special chars */
    strcat( g_preamble_2,
              "\\" );                           /* a backslash char */
    strcat( g_preamble_2,
              "]^_`{|}~" );                     /* more special chars */
    strcat( g_preamble_2,
              "0123456789" );                   /* digits */
    strcat( g_preamble_2,
              "**************************" );   /* twenty six asterisks */
    strcat( g_preamble_2, new_line_s );


    strcpy( g_trailer, g_trailer_start );
    strcat( g_trailer,
        "*****************************************************************" );
    strcat( g_trailer, new_line_s );

    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: skip_header

Description:

  Skip header information when opening a file for read.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

static void skip_header( file_p file_ptr, int* ifail )
    {
    char buffer[max_header_line];
    int chars_read = 0;
    int end_header = 0;
    int first_line = 1;
    while (!end_header)
        {
        /* read from the file */
        read_from_file( file_ptr, buffer, 1, max_header_line, &chars_read, ifail );
        if ( *ifail != FR_no_errors ) return;
        /***
        ***/

        if (strncmp( buffer, g_trailer_start, strlen( g_trailer_start )) == 0)
            {
            /*** this is the end of the header */
            end_header = 1;
            }
        else
        if (first_line
        &&  strncmp( buffer, g_preamble_1, strlen( g_preamble_1 ) ) != 0)
            {
            /*
            rewind the file to the beginning as the header is not there
            (this must be a Parasolid version 1 or Romulus version 6 file
            */
            rewind( file_ptr->stream );
            end_header = 1;
            }
        else
            {
            /*  line skipped  */
            }

        first_line = 0;
        }
    *ifail = FR_no_errors;
    }



/*=============================================================================
                 EXTERNAL ROUTINES
=============================================================================*/



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: StartFileFrustrum (registered as FSTART)

Description:

  Start frustrum; set up file structures if not already done

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void StartFileFrustrum( int* ifail )
    {
    int i;
    *ifail = FR_unspecified;

    for ( i = 0; i < max_open_files; i++ )
      stream_id[i] = 0;

    /* set up the global variables required for writing
    frustrum file headers */
    setup_header();
    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: StopFileFrustrum (registered as FSTOP)

Description:

  Stop frustrum. Does nothing much.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void StopFileFrustrum( int* ifail )
    {
    *ifail = FR_unspecified;

    if ( input_output_buffer != NULL )
        {
        input_output_buflen = 0;
        free(input_output_buffer);
        input_output_buffer = NULL;
        }
      file_p file_ptr = open_files;

      /* while there are still files open - close them down */
      while (file_ptr != NULL)
        {
        fclose( file_ptr->stream );
        if (file_ptr->next == NULL)
          {
          /* free the space used in the file pointer */
          free( file_ptr );
          file_ptr = NULL;
          }
        else
          {
          file_ptr = file_ptr->next;
          free( file_ptr->prev );
          }
        }

      /* reset variables and return values */
      file_count = 0;
      open_files = NULL;

    *ifail = FR_no_errors;
    }




/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: OpenReadFrustrumFile ( registered as FFOPRD )

Description:

  Opens a file for read. A file extension is added to show the guise
  and format of the file. If requested, all the line containing the file
  header will be skipped.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void OpenReadFrustrumFile( const int* guise, const int* format, const char* name, 
		    const int* namlen, const int* skiphd, int* strid,
		    int* ifail )
    {
    char   keyname[max_namelen+1];  /* holds key + null char     */
    char  filename[max_namelen+1];  /* holds key + extension     */
    FILE *stream;
    file_p file_ptr;

    *ifail = FR_unspecified;
    *strid = null_strid;

    /* check that limit hasnt been reached */
    if (file_count == max_open_files)
      {
      *ifail = FR_open_fail;
      return;
      }

    strncpy(  keyname, name, *namlen );
    keyname[*namlen]  = end_of_string_c;

    strncpy( filename, name, *namlen );
    filename[*namlen] = end_of_string_c;

    if ( *guise == FFCSCH )
        {
        for ( int i = 0 ; i < *namlen ; i++ )
            filename[i] = tolower(filename[i]);
			// add (and decode) a prefix to the filename 
			extend_schema_filename(filename);
        }

    {
    /* add the file extension */
    char *gui  = filetype_guise_string( *guise);
    strcat( filename, gui );
    if( *guise != FFCXMO )
      {
      char *fmt = filetype_format_string( *format );
      strcat( filename, fmt );
      }
    }


    check_valid_filename( filename, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }

    /* open file for reading */
    if (*format == FFBNRY)
      /* if binary file is opened with "r" instead of "rb" reading will fail */
      /* with end-of-file error, if it reads byte with value equal to CTRL-Z */
        stream = fopen( filename, g_fopen_mode_read_binary );
    else
        stream = fopen( filename, g_fopen_mode_read_text );

    if (stream == 0)
      {
      *ifail = FR_not_found;
      return;
      }

    new_open_file( stream, *guise, *format, read_access,
                   filename, keyname, &file_ptr, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }


    if (*skiphd == FFSKHD)
      {
      skip_header( file_ptr, ifail );
      if ( *ifail != FR_no_errors )
        {
        return;
        }
      }

    *strid = file_ptr->strid;

    *ifail = FR_no_errors;
    }


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: OpenWriteFrustrumFile ( registered as FFOPWR )

Description:

  Opens file to be written and writes to it the standard file header.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void OpenWriteFrustrumFile( const int* guise, const int* format, const char* name,
		    const int* namlen, const char* pd2hdr, const int *pd2len,
		    int *strid, int *ifail )
    {
    char   keyname[max_namelen+1];  /* holds key + null char    */
    char  filename[max_namelen+1];  /* holds key + extension    */
    FILE *stream;
    file_p file_ptr;

    *ifail = FR_unspecified;
    *strid = null_strid;

    if (file_count == max_open_files)
      {
      *ifail = FR_open_fail;
      return;
      }

    strncpy(  keyname, name, *namlen );
    keyname[*namlen]  = end_of_string_c;

    strncpy( filename, name, *namlen );
    filename[*namlen] = end_of_string_c;

    if ( *guise == FFCSCH )
        {
        // add (and decode) a prefix to the filename  
        extend_schema_filename(filename);
        }

    {
    /* add the file extension */
    char *gui  = filetype_guise_string( *guise );
    strcat( filename, gui );
    if( *guise != FFCXMO && *guise != FFCDBG )
      {
      char *fmt = filetype_format_string( *format );
      strcat( filename, fmt );
      }
    }

    check_valid_filename( filename, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }

    /* open file for writing */
    stream = fopen( filename, g_fopen_mode_write );
	if (stream == 0 ) 
      {
      //*ifail = FR_already_exists;
		*ifail = FR_open_fail;
      return;
      }

    new_open_file( stream, *guise, *format, write_access,
                  filename, keyname, &file_ptr, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }

    if ( *guise == FFCDBG )
		write_xml_header( file_ptr, pd2hdr, *pd2len, ifail );
	else
		write_header( file_ptr, pd2hdr, *pd2len, ifail );

    if ( *ifail != FR_no_errors )
      {
      return;
      }

    *strid = file_ptr->strid;
    *ifail = FR_no_errors;
    }




/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: WriteToFrustrumFile - ( registered as FFWRIT )

Description:

  Write buffer to open file.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void WriteToFrustrumFile( const int* guise, const int* strid, const int* nchars,
		    const char* buffer, int* ifail)
    {
    file_p file_ptr;
    *ifail = FR_unspecified;

    /* find the file info for this stream-id  */
    for ( file_ptr = open_files; file_ptr != NULL; file_ptr = file_ptr->next )
      {
      if (file_ptr->strid == *strid) break;
      }

    if (file_ptr == NULL)
      {
      *ifail = FR_internal_error;
      return;
      }


    /* check file guise */
    if (*guise != file_ptr->guise)
      {
      *ifail = FR_unspecified;
      return;
      }

    /* check access */
    if (file_ptr->access != write_access &&
        file_ptr->access != read_write_access)
      {
      *ifail = FR_unspecified;
      return;
      }


    write_to_file( file_ptr, buffer, 0, *nchars, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }

	/* If we are writing a journal or debug report file then flush the buffer - 
	this to ensure that in the event of crash as much data is preserved as possible */
	if (*guise == FFCJNL || *guise == FFCDBG) 
		fflush( file_ptr->stream );


    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: ReadFromFrustrumFile - ( registered as FFREAD ) 

Description:

  Read buffer from open file.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void ReadFromFrustrumFile( const int* guise, const int* strid, const int* nmax,
		    char* buffer, int* nactual, int* ifail)
    {
    file_p file_ptr;
    int chars_read = 0;
    *ifail = FR_unspecified;
    *nactual = 0;

    /* find the correct file pointer */
    for ( file_ptr = open_files; file_ptr != NULL; file_ptr = file_ptr->next )
      {
      if (file_ptr->strid == *strid) break;
      }
    if (file_ptr == NULL)
        {
        *ifail = FR_internal_error;
        return;
        }


    /* check file guise */
    if (*guise != file_ptr->guise)
      {
      *ifail = FR_unspecified;
      return;
      }

    /* check access */
    if (file_ptr->access != read_access &&
        file_ptr->access != read_write_access)
      {
      *ifail = FR_unspecified;
      return;
      }

    /* read the information from the file */
    read_from_file( file_ptr, buffer, 0, *nmax, &chars_read, ifail );
    if ( *ifail != FR_no_errors )
      {
      return;
      }


    *nactual = chars_read;
    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: CloseFrustrumFile - ( registered as FFCLOS )

Description:

  Close specified file. If a rollback file or the action is abort then
  delete the file.

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void CloseFrustrumFile( const int* guise, const int* strid, const int* action,
		    int* ifail )
    {
    file_p file_ptr;
    char filename[max_namelen+1];
    int delete_it = 0;
    *ifail = FR_unspecified;

    /* find the file info for this stream-id  */
    for ( file_ptr = open_files; file_ptr != NULL; file_ptr = file_ptr->next )
      {
      if (file_ptr->strid == *strid) break;
      }

    if (file_ptr == NULL)
      {
      *ifail = FR_close_fail;
      return;
      }


    if ( file_ptr->access == read_write_access ||
         (file_ptr->access == write_access && *action == FFABOR) )
      {
      delete_it = 1;
      strcpy( filename, file_ptr->name );
      }


    /* close file */
    stream_id[file_ptr->strid - 1] = 0;
    if (fclose( file_ptr->stream ) == EOF)
      {
      *ifail = FR_close_fail;
      return;
      }

    if (file_ptr == open_files)
        open_files = open_files->next;
    else
        file_ptr->prev->next = file_ptr->next;

    if (file_ptr->next != NULL)
        file_ptr->next->prev = file_ptr->prev;

    free( file_ptr );
    file_count--;

    if (delete_it)
      {
      delete_file( filename, ifail );
      if ( *ifail != FR_no_errors )
        {
        return;
        }
      }

    *ifail = FR_no_errors;
    }



/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

Function: AbortFrustrum - ( Registered as FABORT )

Description:

  Aborting a kernel operation. In this implementation, it does nothing

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/

void AbortFrustrum( int* ifail )
    {
    *ifail = FR_no_errors;
    }
