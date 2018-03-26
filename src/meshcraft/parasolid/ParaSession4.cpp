#include <stdio.h>
#include <stdlib.h>

#include "paraSession4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" void FSTART( int *);
extern "C" void FABORT( int *);
extern "C" void FSTOP( int *);
extern "C" void FMALLO( int *, char **, int *);
extern "C" void FMFREE( int *, char **, int *);
extern "C" void GOSGMT( const int *, const int *, const int *, const int *,
                        const double *, const int *, const int *, int *);
extern "C" void GOOPSG( const int *, const int *, const int *, const int *,
                        const double *, const int *, const int *, int *);
extern "C" void GOCLSG( const int *, const int *, const int *, const int *,
                        const double *, const int *, const int *, int *);
extern "C" void GOPIXL( const int *, const double *, const int *, const int *,
                        int *);
extern "C" void GOOPPX( const int *, const double *, const int *, const int *,
                        int *);
extern "C" void GOCLPX( const int *, const double *, const int *, const int *,
                        int *);
extern "C" void FFOPRD( const int *, const int *, const char *, const int *,
                        const int *, int *, int *);
extern "C" void FFOPWR( const int *, const int *, const char *, const int *,
                        const char *, const int *, int *, int *);
extern "C" void FFCLOS( const int *, const int *, const int *, int *);
extern "C" void FFREAD( const int *, const int *, const int *, char *, int *,
                        int *);
extern "C" void FFWRIT( const int *, const int *, const int *, const char *,
                        int *);
extern "C" void FFOPRB( const int *, const int *, const int *, int *, int *,
                        int *);
extern "C" void FFSEEK( const int *, const int *, const int *, int *);
extern "C" void FFTELL( const int *, const int *, int *, int *);
extern "C" void FGCRCU( const char *, int *, int *, int *, int *, double *,
                        int *, double *, int *);
extern "C" void FGCRSU( const char *, int *, int *, int *, int *, double *,
                        int *, double *, int *);
extern "C" void FGEVCU( int *, double *, double *, double *, int *,
                        double *, int *);
extern "C" void FGEVSU( int *, double *, double *, double *, double *,
                        int *, int *, int *, double *, int *);
extern "C" void FGPRCU( int *, double *, double *, double *, int *, int *);
extern "C" void FGPRSU( int *, double *, double *, double *, int *, int *);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSession_c::CSession_c()
{
	parasolidStarted =  false;
	if ( !Start() )
		exit(0);
}

CSession_c::~CSession_c()
{
	Stop();
}

// Starts up Parasolid Session, returns true is successful, false otherwise.
bool CSession_c::Start()
{
  bool ok = true;

  if(!parasolidStarted){
    // starting with Parasolid v7.1 need to register the frustrum

	// **** Register frustrum functions **** //
	// Note: the GO functions are included
    PK_SESSION_frustrum_t fru;
    PK_SESSION_frustrum_o_m( fru );
	fru.fstart = FSTART;
	fru.fabort = FABORT;
	fru.fstop  = FSTOP;
	fru.fmallo = FMALLO;
	fru.fmfree = FMFREE;
	fru.gosgmt = GOSGMT;
	fru.goopsg = GOOPSG;
	fru.goclsg = GOCLSG;
	fru.gopixl = GOPIXL;
	fru.gooppx = GOOPPX;
	fru.goclpx = GOCLPX;
	fru.ffoprd = FFOPRD;
	fru.ffopwr = FFOPWR;
	fru.ffclos = FFCLOS;
	fru.ffread = FFREAD;
	fru.ffwrit = FFWRIT;
	fru.ffoprb = FFOPRB;
	fru.ffseek = FFSEEK;
	fru.fftell = FFTELL;
	fru.fgcrcu = FGCRCU;
	fru.fgcrsu = FGCRSU;
	fru.fgevcu = FGEVCU;
	fru.fgevsu = FGEVSU;
	fru.fgprcu = FGPRCU;
	fru.fgprsu = FGPRSU;
    
    PK_ERROR_code_t pkErr;
    pkErr = PK_SESSION_register_frustrum(&fru);
    
	// **** Register Delta Frustrum **** //

	// **** Register Error Handler Frustrum ***** //
	 PK_ERROR_frustrum_t errhandler;
	 errhandler.handler_fn = PKerrorHandler;
	 PK_ERROR_code_t err_reg_err_fru = PK_ERROR_register_callbacks(errhandler);
	 //assert( err_reg_err_fru == PK_ERROR_no_errors );

	// **** Starts the modeller **** //
    PK_SESSION_start_o_t sessionData;
    PK_SESSION_start_o_m(sessionData);
    //sessionData.user_field = 1;			// Set the user field size to be 1
    sessionData.journal_file = "";

    pkErr = PK_SESSION_start(&sessionData);
    
    if (pkErr != PK_ERROR_no_errors) {
    	printf("CSession_c::Start - could not start session\n");
    	return false;
    }
    
	parasolidStarted = true ;  

	char envVariable[]={"P_SCHEMA"};
	char *envValue = getenv(envVariable);	
	if( !envValue ) {
		//_putenv( "P_SCHEMA=c:\\myDev\\MeshWorks\\2012\\thirdpty\\schema" );
		_putenv( "P_SCHEMA=..\\schema" );
	}
  }

  return ok;
}

bool CSession_c::Stop()
{
	PK_SESSION_stop();
	return true;
}

PK_ERROR_code_t CSession_c::PKerrorHandler( PK_ERROR_sf_t* error )
{
	//printf("Parasolid Reports Error in function %s\n", error->function);
	//printf("\tError: %d  %s\n", error->code, error->code_token);
	//CString text;
	//text.Format( "PK error: %s returned %s.", error->function, 
	//	error->code_token );
	//AfxMessageBox( text.GetBuffer( 0 ) );
	return error->code;
}
