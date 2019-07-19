//========== Copyright ToonBoom Technologies Inc. 2001 ============
//
// SOURCE FILE : UT_String.h
// MODULE NAME : 
// DESCRIPTION : 
//
//=================================================================
// Author : Marc Begin
// Modif  : 
//
// Creation Date		 : 2001-9-21
//========================VSS Auto=================================
// $Date: 2006/06/14 20:31:21 $
// $Author: aurelio $
//=================================================================
// REVISION: 
// 
//=========== Copyright ToonBoom Technologies Inc. 2001  ==========
#ifndef CORE__H_UT_STRING
#define CORE__H_UT_STRING

#define NOMINMAX

#include "MEM_Override.h"
#include "PL_Configure.h"

#include <stdexcept>
#include <string>
#include <algorithm>
#include <stdio.h>

#if defined(TARGET_IOS) || defined(TARGET_MACOS)
 #include <CoreFoundation/CFBase.h>
 #include <CoreFoundation/CFString.h>
#endif

#include <stdio.h>
#include <string.h>

#if (defined TARGET_WIN32 )
  typedef wchar_t  UniChar_t ;
#else
typedef unsigned short UniChar_t;

namespace std
{
  template<> struct char_traits<UniChar_t>
    {
    typedef UniChar_t	char_type;  //  type of character
    
    static void assign( char_type &c1, const char_type &c2 )
    {   c1 = c2;	}
    
    //  interger representation of characters
    typedef unsigned int			int_type;
    
    static char_type to_char_type( const int_type &c )
    {
       return char_type( c );
    }
    
    static int_type to_int_type( const char_type &c )
    {
       return int_type( c );
    }
    
    static bool eq_int_type( const int_type &c1, const int_type &c2 )
    {
       return c1 == c2;
    }
    
    //  char_type comparisons
    static bool eq( const char_type &c1, const char_type &c2 )
    {
       return c1 == c2;
    }
    
    static bool lt( const char_type &c1, const char_type &c2 )
    {
       return c1 < c2;
    }

    //  operation on s[n] arrays
    static char_type *move( char_type *s1, const char_type *s2, int_type n )
    {
      return (char_type *)memmove( s1, s2, n * sizeof( char_type ) );
    }
    
    static char_type *copy( char_type *s1, const char_type *s2, size_t n )
    {
       return (char_type *) memcpy(s1, s2, n * sizeof( char_type ) );
    }
    
    static char_type *assign( char_type *s, size_t n, char_type a )
    {
      for ( char_type *p = s; p < s + n; ++p )
        *p = a;
      return s;
    }
    
    static int compare( const char_type *s1, const char_type *s2, size_t n )
    {
      for ( size_t i = 0; i < n; ++i )
      if ( s1[i] != s2[i] )
        return s1[i] < s2[i] ? -1 : 1;
      return 0;
    }

    static size_t length( const char_type *s )
    {
      const char_type *p = s;
      while ( *p ) ++p;
      return (p - s);
    }

    static const char_type *find( const char_type *s, size_t n, const char_type &a )
    {
      for ( const char_type *p = s; size_t(p - s) < n; ++p )
        if ( *p == a )
          return p;
      return 0;
    }
    
    //  I/O related:
    typedef streamoff   off_type;
    typedef streampos   pos_type;
    typedef mbstate_t	state_type;
      
    static int_type eof()
    {
      return static_cast<int_type>( -1 );
    }

    static int_type not_eof( const int_type &c )
    {
      return eq_int_type( c, eof() ) ? 0 : c;
    }
  };
}
#endif

class   UT_String
{
  MEM_OVERRIDE

  public:
    // types:
    typedef std::basic_string<UniChar_t>::size_type					size_type;
    typedef std::basic_string<UniChar_t>::iterator					iterator;
    typedef std::basic_string<UniChar_t>::const_iterator			const_iterator;
    typedef std::basic_string<UniChar_t>::reverse_iterator			reverse_iterator;
    typedef std::basic_string<UniChar_t>::const_reverse_iterator	const_reverse_iterator;
    typedef std::basic_string<UniChar_t>::reference					reference;
    typedef std::basic_string<UniChar_t>::const_reference			const_reference;
    typedef std::basic_string<UniChar_t>::value_type				value_type;
    typedef std::basic_string<UniChar_t>::pointer					pointer;
    typedef std::basic_string<UniChar_t>::const_pointer				const_pointer;
    typedef std::basic_string<UniChar_t>::difference_type			difference_type;
    
    static const size_type npos = size_type(-1);
    
    //	contructors:
    UT_String(){}
    UT_String( const char *s );
    UT_String( const UniChar_t *s ) : m_string( s ){}
    UT_String( const UT_String &str, size_type pos = 0, size_type n = npos ) : m_string( str.m_string, pos, n ){}
    UT_String( size_type n, UniChar_t c ) : m_string( n, c ){}
    UT_String( const UniChar_t *s, size_type n ) : m_string( s, n ){}
    UT_String( const_iterator first, const_iterator last ) : m_string( first, last ){}
    ~UT_String(){}

    inline UT_String	&operator=( const std::basic_string<UniChar_t> &str )	{	m_string.assign( str ); return *this;	}
    inline UT_String	&operator=( const UT_String &str )						{	m_string.assign( str.m_string ); return *this;	}
    inline UT_String	&operator=( const UniChar_t *s )						{	m_string.assign( s ); return *this;	}
    inline UT_String	&operator=( UniChar_t c )								{	m_string.assign( 1, c ); return *this;	}

    // iterators:
    inline iterator			begin()			{	return m_string.begin();	}
    inline const_iterator	begin() const	{	return m_string.begin();	}
    inline iterator			end()			{	return m_string.end();	}
    inline const_iterator	end() const		{	return m_string.end();	}

    inline reverse_iterator			rbegin()		{	return m_string.rbegin();	}
    inline const_reverse_iterator	rbegin() const	{	return m_string.rbegin();	}
    inline reverse_iterator			rend()			{	return m_string.rend();	}
    inline const_reverse_iterator	rend() const	{	return m_string.rend();	}
    
    // capacity:
    inline size_type	size() const				{	return m_string.size();	}
    inline size_type	length() const				{	return m_string.length();	}
    inline size_type	max_size() const			{	return m_string.max_size();	}
    inline void	resize( size_type n, UniChar_t c )	{	m_string.resize( n, c );	}
    inline void	resize( size_type n )				{	m_string.resize( n );	}
    inline size_type	capacity() const			{	return m_string.capacity();	}
    inline void	reserve( size_type n = 0 )			{	m_string.reserve( n );	}
    inline void	clear()								{	m_string.clear();	}
    inline bool	empty() const						{	return m_string.empty();	}
    
    // element access:
    inline const_reference operator[](size_type pos) const	{	return m_string[pos];	}
    inline reference       operator[](size_type pos)		{	return m_string[pos];	}
    inline const_reference at( size_type pos ) const		{	return m_string.at( pos );	}
    inline reference       at( size_type pos )				{	return m_string.at( pos );	}
    // modifiers:
    inline UT_String	&operator+=( const UT_String &str )							{	m_string.append( str.m_string ); return *this;	}
    inline UT_String	&operator+=( const UniChar_t *s )							{	m_string.append( s ); return *this;	}
    inline UT_String	&operator+=( UniChar_t c )									{	m_string.append( 1, c ); return *this;	}
    inline UT_String	&append( const UT_String &str )								{	m_string.append( str.m_string ); return *this;	}
    inline UT_String	&append( const UT_String &str, size_type pos, size_type n )	{	m_string.append( str.m_string, pos, n ); return *this;	}
    inline UT_String	&append( const UniChar_t *s, size_type n )					{	m_string.append( s, n ); return *this;	}
    inline UT_String	&append( const UniChar_t *s )								{	m_string.append( s ); return *this;	}
    inline UT_String	&append( size_type n, UniChar_t c )							{	m_string.append( n, c ); return *this;	}
    inline UT_String	&append( const_iterator first, const_iterator last )		{	m_string.append( first, last ); return *this;	}
    inline void	push_back( UniChar_t c )											{	m_string.push_back( c );	}

    inline UT_String	&assign( const UT_String &str )								{	m_string.assign( str.m_string ); return *this;	}
    inline UT_String	&assign( const UT_String &str, size_type pos, size_type n )	{	m_string.assign( str.m_string, pos, n ); return *this;	}
    inline UT_String	&assign( const UniChar_t *s, size_type n )					{	m_string.assign( s, n ); return *this;	}
    inline UT_String	&assign( const UniChar_t *s )								{	m_string.assign( s ); return *this;	}
    inline UT_String	&assign( size_type n, UniChar_t c )							{	m_string.assign( n, c ); return *this;	}
    inline UT_String	&assign( const_iterator first, const_iterator last )		{	m_string.assign( first, last ); return *this;	}
    
    inline UT_String	&insert( size_type pos1, const UT_String &str )									{	m_string.insert( pos1, str.m_string ); return *this;	}
    inline UT_String	&insert( size_type pos1, const UT_String &str, size_type pos2, size_type n )	{	m_string.insert( pos1, str.m_string, pos2, n ); return *this;	}
    inline UT_String	&insert( size_type pos, const UniChar_t *s, size_type n )						{	m_string.insert( pos, s, n ); return *this;	}
    inline UT_String	&insert( size_type pos, const UniChar_t *s )									{	m_string.insert( pos, s ); return *this;	}
    inline UT_String	&insert( size_type pos, size_type n, UniChar_t c )								{	m_string.insert( pos, n, c ); return *this;	}

    inline UT_String	&erase( size_type pos = 0, size_type n = npos )	{	m_string.erase( pos, n ); return *this;	}
    inline iterator		erase( iterator position )						{	return m_string.erase( position );	}
    inline iterator		erase( iterator first, iterator last )			{	return m_string.erase( first, last );	}

    inline UT_String	&replace( size_type pos, size_type n, const UT_String &str )									{	m_string.replace( pos, n, str.m_string ); return *this;	}
    inline UT_String	&replace( size_type pos1, size_type n1, const UT_String &str, size_type pos2, size_type n2 )	{	m_string.replace( pos1, n1, str.m_string, pos2, n2 ); return *this;	}
//	basic_string& replace(size_type pos, size_type n1, const UniChar_t *s, size_type n2);
    inline UT_String	&replace( size_type pos, size_type n, const UniChar_t *s )										{	m_string.replace( pos, n, s ); return *this;	}
//	basic_string& replace(size_type pos, size_type n1, size_type n2, UniChar_t c);
    inline UT_String	&replace( iterator first, iterator last, const UT_String &str )									{	m_string.replace( first, last, str.m_string ); return *this;	}
//	basic_string& replace(iterator i1, iterator i2, const UniChar_t *s, size_type n);
//	basic_string& replace(iterator i1, iterator i2, const UniChar_t *s);
//	basic_string& replace(iterator i1, iterator i2, size_type n, UniChar_t c);

    inline size_type	copy( UniChar_t *s, size_type n, size_type pos = 0 ) const	{	return m_string.copy( s, n, pos );	}
    inline void			swap( UT_String &str )										{	m_string.swap( str.m_string );	}
    
    // string operations:
    inline const UniChar_t	*c_str() const	{	return m_string.c_str();	}
    inline const UniChar_t	*data() const	{	return m_string.data();	}
    
    inline size_type	find( const UT_String &str, size_type pos = 0 ) const			{	return m_string.find( str.m_string, pos );	}
    inline size_type	find( const UniChar_t *s, size_type pos, size_type n ) const	{	return m_string.find( s, pos, n );	}
    inline size_type	find( const UniChar_t *s, size_type pos = 0 ) const				{	return m_string.find( s, pos );	}
    inline size_type	find( UniChar_t c, size_type pos = 0 ) const					{	return m_string.find( c, pos );	}
    inline size_type	rfind( const UT_String &str, size_type pos = npos ) const		{	return m_string.rfind( str.m_string, pos );	}
    inline size_type	rfind( const UniChar_t *s, size_type pos, size_type n ) const	{	return m_string.rfind( s, pos, n );	}
    inline size_type	rfind( const UniChar_t *s, size_type pos = npos ) const			{	return m_string.rfind( s, pos );	}
    inline size_type	rfind( UniChar_t c, size_type pos = npos ) const				{	return m_string.rfind( c, pos );	}

    inline size_type	find_first_of( const UT_String &str, size_type pos = 0 ) const				{	return m_string.find_first_of( str.m_string, pos );	}
    inline size_type	find_first_of( const UniChar_t *s, size_type pos, size_type n ) const		{	return m_string.find_first_of( s, pos, n );	}
    inline size_type	find_first_of( const UniChar_t *s, size_type pos = 0 ) const				{	return m_string.find_first_of( s, pos );	}
    inline size_type	find_first_of( UniChar_t c, size_type pos = 0 ) const						{	return m_string.find_first_of( c, pos );	}
    inline size_type	find_last_of( const UT_String &str, size_type pos = npos ) const			{	return m_string.find_last_of( str.m_string, pos );	}
    inline size_type	find_last_of( const UniChar_t *s, size_type pos, size_type n ) const		{	return m_string.find_last_of( s, pos, n );	}
    inline size_type	find_last_of( const UniChar_t *s, size_type pos = npos ) const				{	return m_string.find_last_of( s, pos );	}
    inline size_type	find_last_of( UniChar_t c, size_type pos = npos ) const						{	return m_string.find_last_of( c, pos );	}
    inline size_type	find_first_not_of( const UT_String &str, size_type pos = 0 ) const			{	return m_string.find_first_not_of( str.m_string, pos );	}
    inline size_type	find_first_not_of( const UniChar_t *s, size_type pos, size_type n ) const	{	return m_string.find_first_not_of( s, pos, n );	}
    inline size_type	find_first_not_of( const UniChar_t *s, size_type pos = 0 ) const			{	return m_string.find_first_not_of( s, pos );	}
    inline size_type	find_first_not_of( UniChar_t c, size_type pos = 0 ) const					{	return m_string.find_first_not_of( c, pos );	}
    inline size_type	find_last_not_of( const UT_String &str, size_type pos = npos ) const		{	return m_string.find_last_not_of( str.m_string, pos );	}
    inline size_type	find_last_not_of( const UniChar_t *s, size_type pos, size_type n ) const	{	return m_string.find_last_not_of( s, pos, n );	}
    inline size_type	find_last_not_of( const UniChar_t *s, size_type pos = npos ) const			{	return m_string.find_last_not_of( s, pos );	}
    inline size_type	find_last_not_of( UniChar_t c, size_type pos = npos ) const					{	return m_string.find_last_not_of( c, pos );	}
    
    UT_String	substr( size_type pos = 0, size_type n = npos ) const;
    
    inline int	compare( const UT_String &str ) const			{	return m_string.compare( str.m_string );	}
    int			compare_nocase( const UT_String &str ) const;
    inline int	compare( const UniChar_t *s ) const				{	return m_string.compare( s );	}
    int			compare_nocase( const UniChar_t *s ) const;

    //	extensions
    std::string	utf8() const;
    UT_String	upper() const;
    UT_String	lower() const;

      // Trims particular characters from the beginning of the string.  When called
      // with no parameters, trims whitespace (newline, space, tab)
      UT_String &TrimLeft();
      UT_String &TrimLeft(const UT_String &str);
      UT_String &TrimLeft(const UniChar_t *s);
      UT_String &TrimLeft(UniChar_t c);
    
      // Trims particular characters from the end of the string.  When called
      // with no parameters, trims whitespace (newline, space, tab)
      UT_String &TrimRight();
      UT_String &TrimRight(const UT_String &str);
      UT_String &TrimRight(const UniChar_t *s);
      UT_String &TrimRight(UniChar_t c);

    //	Convert a UniChar string to a CFString (for Carbon and Cocoa compatibility)
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
    CFStringRef			ToCFString() const;
    CFMutableStringRef	ToCFMutableString() const;
    //	Convert a CFString to a UniChar string (for Carbon and Cocoa compatibility)
    explicit UT_String( const CFStringRef src );
#endif

  private:
    std::basic_string<UniChar_t>	m_string;
};

inline UT_String UT_String::substr( size_type pos, size_type n ) const
{
  if ( pos > m_string.size() )
    fprintf( stderr, "UT_String::substr pos out of range" );

  size_type rlen = (std::min)( n, size() - pos );
  return UT_String( *this, pos, rlen );
}

inline bool operator==( const UT_String &lhs, const UT_String &rhs )
{
  return lhs.compare(rhs) == 0;
}

inline bool operator!=( const UT_String &lhs, const UT_String &rhs )
{
  return lhs.compare(rhs) != 0;
}

inline bool operator<( const UT_String &lhs, const UT_String &rhs )
{
  return lhs.compare(rhs) < 0;
}

inline UT_String operator+( const UT_String &lhs, const UT_String &rhs )
{
  UT_String   str( lhs );
  str.append( rhs );
  return str;
}

typedef const char **UT_ResourceId;



//	This FormatMessage accept %f and format them with user
//	preferences for decimal separator
UT_String   UT_FormatMessage(const char* sFormatString, ... );
UT_String   UT_FormatMessage(const UniChar_t* sFormatString, ... );

//	parse a float with (,) or (.) as decimal separator
double   UT_W2Float( const UT_String &s );
int   UT_W2Int( const UT_String &s );

// Convert a 'char *' to a UT_String
void   UT_A2W( const char *pSrc, UT_String &dst);

// Convert a string to a UT_String
void   UT_S2W( const std::string &src, UT_String &dst);

// Convert a UT_String to a string.
void   UT_W2S( const UT_String &src, std::string &dst );

// Convert a 'char *' to a UT_String
UT_String   UT_A2W( const char *pSrc );

// Convert a string to a UT_String
UT_String   UT_S2W( const std::string &src );

// Convert a UT_String to a string.
std::string   UT_W2S( const UT_String &src );

// Case-insensitive string comparison
int   UT_CompareNoCase( const UT_String& rhs, const UT_String& lhs );

// Case-sensitive string comparison
int   UT_Compare( const UT_String& rhs, const UT_String& lhs );

#endif
