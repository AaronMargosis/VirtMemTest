//	SharedMem.cpp

#include "stdafx.h"
#include "SharedMem.h"


SharedMem::SharedMem()
:	m_hMap( NULL ),
	m_lpMapAddr( NULL ),
	m_errCode( 0 )
{
}


SharedMem::~SharedMem()
{
	Close() ;
}


LPVOID SharedMem::Create( LPCWSTR name,DWORD dwSizeLow /*= 512*/, DWORD  dwSizeHigh /*= 0*/ )
{
	Close() ;
	m_name = name;

	m_hMap = CreateFileMappingW(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		dwSizeHigh, dwSizeLow, 
		m_name.c_str() ) ;

	if ( m_hMap )
	{
		m_lpMapAddr = MapViewOfFile(
			m_hMap,
			FILE_MAP_ALL_ACCESS,
			0, 0, 0 ) ;
	}

	m_errCode = GetLastError() ;

	return m_lpMapAddr ;
}


LPVOID SharedMem::Open( LPCWSTR name, DWORD dwDesiredAccess /* = FILE_MAP_ALL_ACCESS */ )
{
	Close() ;
	m_name = name ;

	m_hMap = OpenFileMappingW(
		dwDesiredAccess,
		FALSE,
		m_name.c_str() ) ;

	if ( m_hMap )
	{
		m_lpMapAddr = MapViewOfFile(
			m_hMap,
			dwDesiredAccess,
			0, 0, 0 ) ;
	}

	m_errCode = GetLastError() ;

	return m_lpMapAddr ;
}


void SharedMem::Close()
{
	if ( m_lpMapAddr )
		UnmapViewOfFile( m_lpMapAddr ) ;
	if ( m_hMap )
		CloseHandle( m_hMap ) ;
	m_name = L"" ;
	m_lpMapAddr = NULL ;
	m_hMap = NULL ;
}

