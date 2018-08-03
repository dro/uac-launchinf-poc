#pragma once


#define PHNT_VERSION PHNT_THRESHOLD // Windows 10

#include <phnt_windows.h>
#include <phnt.h>

struct detail_spoof_data {
	void                 *m_page;
	SIZE_T                m_page_size;
	UNICODE_STRING        m_backup_imagep;
	UNICODE_STRING        m_backup_full_dllname;
	UNICODE_STRING        m_backup_base_dllname;
	LDR_DATA_TABLE_ENTRY *m_backup_entry;
};

struct detail_pump_data {
	int a;
};

BOOLEAN detail_spoof_peb( struct detail_spoof_data *data, const wchar_t *path, const wchar_t *file );
void    detail_restore_peb( struct detail_spoof_data *data );

BOOLEAN detail_create_inf( const wchar_t *name, const char *buf, size_t size );
BOOLEAN detail_delete_inf( const wchar_t *name );