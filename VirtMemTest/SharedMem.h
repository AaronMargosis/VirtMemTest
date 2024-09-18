//	SharedMem.h
#pragma once


// ----------------------------------------------------------------------

//TODO:  Provide a way for size to be known.
//	E.g., create block with requested size + sizeof(size_t), write the
//	size at the beginning of the block, and return the address
//	immediately following it.


/// <summary>
/// Class that encapsulates a named or unnamed file mapping backed by the system paging file.
/// Although its destructor releases allocated resources, this app can still leak them.
/// </summary>
class SharedMem
{
public:
	SharedMem() ;
	virtual ~SharedMem() ;

	LPVOID Create( LPCWSTR name, DWORD dwSizeLow = 512, DWORD  dwSizeHigh = 0) ;
	LPVOID Open( LPCWSTR name, DWORD dwDesiredAccess = FILE_MAP_ALL_ACCESS ) ;
	void Close() ;

	UINT ErrorCode() const
		{	return m_errCode ; }

	LPVOID Address() const
		{	return m_lpMapAddr ; }

	const wstring & Name() const
		{	return m_name ; }

	HANDLE hMap() const
		{	return m_hMap ; }

private:
	HANDLE m_hMap ;
	LPVOID m_lpMapAddr ;
	wstring  m_name ;
	DWORD  m_errCode ;

private:
	//	Not implemented:
	SharedMem( const SharedMem & ) = delete;
	SharedMem & operator = ( const SharedMem & ) = delete ;
};
