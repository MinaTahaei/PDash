//========== Copyright ToonBoom Technologies Inc. 2001 ============
//
// SOURCE FILE : UT_String.cpp
// MODULE NAME : 
// DESCRIPTION : 
//
//=================================================================
// Author : Marc Begin
// Modif  : 
//
// Creation Date		 : 2001-9-21
//========================VSS Auto=================================
// $Revision: 1.8.22.1.2.4 $
// $Date: 2006/11/06 02:59:54 $
// $Author: begin $
//=================================================================
// REVISION: 
// 
//=========== Copyright ToonBoom Technologies Inc. 2001  ==========

//=================================================================
//						INCLUDE
//=================================================================


//=================================================================
//						DEFINE
//=================================================================
#include "UT_String.h"
#include <stdlib.h>
#include "UT_CVTUTF.h"

using namespace std;

UT_String::UT_String( const char *s )
{
  UTF16	buffer[64];
  UTF8	*sourceStart = (UTF8 *)s;
  UTF8	*sourceEnd = (UTF8 *)s + strlen( s );
  UTF16	*targetStart, *targetEnd = buffer + 64;
  
  ConversionResult	res;
  do
  {
    targetStart = buffer;
    res = ConvertUTF8toUTF16( &sourceStart, sourceEnd, &targetStart, targetEnd );
    sourceStart--;
    m_string.append( buffer, targetStart );
  }
  while ( res == targetExhausted );
}

int UT_String::compare_nocase( const UT_String &str ) const
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFStringRef	theString1 = CFStringCreateWithCharactersNoCopy( 0, m_string.data(), m_string.length(), kCFAllocatorNull );
  CFStringRef	theString2 = CFStringCreateWithCharactersNoCopy( 0, str.m_string.data(), str.m_string.length(), kCFAllocatorNull );
  CFComparisonResult	result = ::CFStringCompare( theString1, theString2, kCFCompareCaseInsensitive );
  ::CFRelease( theString1 );
  ::CFRelease( theString2 );
  return result;
#else
  int index = 0;
  int length0 = (int)m_string.length();
  int length1 = (int)str.m_string.length();
  for(index = 0 ; index < length0 && index < length1; ++index)
  {
    unsigned short c0 = m_string[index];
    unsigned short c1 = str.m_string[index];
    if (c0 == c1)
      continue;
    if (c0 < 128 && c1 < 128)
    {
      c0 = toupper(c0);
      c1 = toupper(c1);      
    }
    if (c1 == c0)
      continue;

    return c0 < c1 ? -1 : 1;
  }
  if (length0 == length1)
    return 0;
  return length0 < length1 ? -1 : 1;
#endif
}

//	extensions
string UT_String::utf8() const
{
  string	s;
  
  UTF8	buffer[64];
  UTF16	*sourceStart = (UTF16 *)m_string.data();
  UTF16	*sourceEnd = (UTF16 *)m_string.data() + m_string.length();
  UTF8	*targetStart, *targetEnd = buffer + 64;
  
  ConversionResult	res;
  do
  {
    targetStart = buffer;
    res = ConvertUTF16toUTF8( &sourceStart, sourceEnd, &targetStart, targetEnd );
    sourceStart--;
    s.append( (string::pointer)buffer, (string::pointer)targetStart );
  }
  while ( res == targetExhausted );
  
  return s;
}

UT_String UT_String::upper() const
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFMutableStringRef theString = ToCFMutableString();
  ::CFStringUppercase( theString, 0 );
  UT_String   s( theString );
  ::CFRelease( theString );
  return s;
#else
  UT_String s = *this;
  int length = (int)s.m_string.length();
  int index = 0;
  for( ; index < length ; ++index)
  {
    if (s.m_string[index] < 128)
      s.m_string[index] = toupper(s.m_string[index]);
  }
  return s;
#endif
}

UT_String UT_String::lower() const
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFMutableStringRef theString = ToCFMutableString();
  ::CFStringLowercase( theString, 0 );
  UT_String   s( theString );
  ::CFRelease( theString );
  return s;
#else
  UT_String s = *this;
  int length = (int)s.m_string.length();
  int index = 0;
  for( ; index < length ; ++index)
  {
    if (s.m_string[index] < 128)
      s.m_string[index] = tolower(s.m_string[index]);
  }
  return s;
#endif
}

UT_String &UT_String::TrimLeft()
{
  return TrimLeft(" \t\n");
}

UT_String &UT_String::TrimLeft(const UT_String &str)
{
  return TrimLeft(str.c_str());
}

UT_String &UT_String::TrimLeft(const UniChar_t *s)
{
  size_type pos = find_first_not_of(s);
  if (pos == npos)
    clear();
  else
    erase(0, pos);
  return *this;
}

UT_String &UT_String::TrimLeft(UniChar_t c)
{
  size_type pos = find_first_not_of(c);
  if (pos == npos)
    clear();
  else
    erase(0, pos);
  return *this;
}

UT_String &UT_String::TrimRight()
{
  return TrimRight(" \t\n");
}

UT_String &UT_String::TrimRight(const UT_String &str)
{
  return TrimRight(str.c_str());
}

UT_String &UT_String::TrimRight(const UniChar_t *s)
{
  size_type pos = find_last_not_of(s);
  if (pos == npos)
    clear();
  else
    erase(pos + 1, npos);
  return *this;
}

UT_String &UT_String::TrimRight(UniChar_t c)
{
  size_type pos = find_last_not_of(c);
  if (pos == npos)
    clear();
  else
    erase(pos + 1, npos);
  return *this;
}

#if defined(TARGET_IOS) || defined(TARGET_MACOS)

//	Convert a UniChar string to a CFString (for Carbon and Cocoa compatibility)
CFStringRef UT_String::ToCFString() const
{
  return CFStringCreateWithCharacters( kCFAllocatorDefault, m_string.data(), m_string.length() );
}

CFMutableStringRef UT_String::ToCFMutableString() const
{
  CFMutableStringRef	cfstring = CFStringCreateMutable( kCFAllocatorDefault, 0 );
  CFStringAppendCharacters( cfstring, m_string.data(), m_string.length() );
  return cfstring;
}

//	Convert a CFString to a UniChar string (for Carbon and Cocoa compatibility)
UT_String::UT_String( const CFStringRef src )
{
  CFStringInlineBuffer	inlineBuf;
  CFIndex	length = CFStringGetLength( src );
  CFStringInitInlineBuffer( src, &inlineBuf, CFRangeMake( 0, length ) );
  
  m_string.reserve( length );
  
  for ( CFIndex i = 0; i < length; ++i )
    m_string.push_back( CFStringGetCharacterFromInlineBuffer( &inlineBuf, i ) );
}

#endif

#if 0
#pragma mark-
#endif

//========================  FUNCTION DECLARATION  =====================
// FCT NAME :    UT_A2W
//---------------------------------------------------------------------
/// \param pSrc 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Convert ascii to a 'wstring'.
//=====================================================================
void UT_A2W( const char *pSrc, UT_String &dst)
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFStringRef	strref = CFStringCreateWithCStringNoCopy( 0, pSrc, CFStringGetSystemEncoding(), kCFAllocatorNull );
  dst = UT_String( strref );
  CFRelease( strref );
  
  size_t uLen = strlen( pSrc );
  dst.resize( uLen );
  UT_String::iterator it = dst.begin();
  
  while( *pSrc )
  {
    *it = wchar_t( *pSrc );
    ++it;
    ++pSrc;
  }
#else
   dst = UT_String();
  if (!pSrc)
    return;
  while (*pSrc)
  {
    dst.push_back(UniChar_t(*pSrc));
    ++pSrc;
  }

#endif
}

//========================  METHOD DECLARATION  =======================
// METHOD NAME : UT_S2W
//---------------------------------------------------------------------
/// \param src 
/// \param dst 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Convert a 'string' to a 'wstring'.
//=====================================================================
void UT_S2W( const string &src, UT_String &dst)
{
  UT_A2W( src.c_str(), dst );
}


//========================  METHOD DECLARATION  =======================
// METHOD NAME : UT_W2S
//---------------------------------------------------------------------
/// \param src 
/// \param dst 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Convert a 'wstring' to a string.
//=====================================================================
void UT_W2S( const UT_String &src, string &dst )
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFStringRef	strref = CFStringCreateWithCharactersNoCopy( 0, src.data(), src.length(), kCFAllocatorNull );
  CFIndex	usedBufLen;
  CFStringGetBytes( strref, CFRangeMake( 0, src.length() ), CFStringGetSystemEncoding(), '?', false, NULL, 0, &usedBufLen );
  char *buffer = (char*)MEM_ALLOC(usedBufLen * sizeof(char));
  CFStringGetBytes( strref, CFRangeMake( 0, src.length() ), CFStringGetSystemEncoding(), '?', false, (UInt8 *) buffer, usedBufLen, &usedBufLen );
  CFRelease( strref );
  dst.assign( buffer, usedBufLen );
  MEM_DEALLOC(buffer);
  if ( ! dst.empty() )
    dst.erase(dst.begin(), dst.end());
  copy( src.begin(), src.end(), back_inserter( dst ));
#else
#if !(defined(TARGET_WIN32) || (defined(TARGET_WEBGL))) 
  dst = string();
  if (src.length() < 1)
  {
    return;
  }
  unsigned char *dstC = (unsigned char*)MEM_ALLOC(6 * src.length() * sizeof(unsigned char));
  unsigned char *initial = dstC;
  UTF16 *srcC = const_cast<UTF16*>(src.c_str());
  ConvertUTF16toUTF8(&srcC, srcC + src.length(), &dstC, dstC + 6 * src.length());
  dst.append((const char *)dstC);
  MEM_DEALLOC(initial);
#else // TARGET_WIN32
  (void)src;
  (void)dst;
#endif
#endif
}



//========================  METHOD DECLARATION  =======================
// METHOD NAME : UT_A2W
//---------------------------------------------------------------------
/// \param pSrc 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Slow version - convert ascii string to UT_String
//=====================================================================
UT_String UT_A2W( const char *pSrc )
{
  UT_String wDst;
  UT_A2W( pSrc, wDst );
  return wDst;
}


//========================  METHOD DECLARATION  =======================
// METHOD NAME : UT_S2W
//---------------------------------------------------------------------
/// \param src 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// slow version - Convert a string to a UT_String
//=====================================================================
UT_String UT_S2W( const string &src )
{	
  UT_String wDst;
  UT_S2W( src, wDst );
  return wDst;
}


//========================  METHOD DECLARATION  =======================
// METHOD NAME : UT_W2S
//---------------------------------------------------------------------
/// \param src 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// slow version -convert wide string to string
//=====================================================================
string UT_W2S( const UT_String &src )
{
  string sStr;
  UT_W2S( src, sStr );
  return sStr;
}

//========================  FUNCTION DECLARATION  =====================
// FCT NAME :    CompareNoCase
//---------------------------------------------------------------------
/// \param rhs 
/// \param lhs 
/// \return 
//---------------------------------------------------------------------
// DESCRIPTION :
/// Make a lexicographical comparaison between rhs and lhs. 
///	
//=====================================================================
int UT_CompareNoCase( const UT_String& rhs, const UT_String& lhs )
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFStringRef	theString1 = CFStringCreateWithCharactersNoCopy( 0, rhs.data(), rhs.length(), kCFAllocatorNull );
  CFStringRef	theString2 = CFStringCreateWithCharactersNoCopy( 0, lhs.data(), lhs.length(), kCFAllocatorNull );
  CFComparisonResult	result = CFStringCompare( theString1, theString2, kCFCompareCaseInsensitive+kCFCompareLocalized );
  CFRelease( theString1 );
  CFRelease( theString2 );
  return result;
#else
   return rhs.compare_nocase(lhs);
#endif
}

int UT_Compare( const UT_String& rhs, const UT_String& lhs )
{
#if defined(TARGET_IOS) || defined(TARGET_MACOS)
  CFStringRef	theString1 = CFStringCreateWithCharactersNoCopy( 0, rhs.data(), rhs.length(), kCFAllocatorNull );
  CFStringRef	theString2 = CFStringCreateWithCharactersNoCopy( 0, lhs.data(), lhs.length(), kCFAllocatorNull );
  CFComparisonResult	result = CFStringCompare( theString1, theString2, kCFCompareLocalized );
  CFRelease( theString1 );
  CFRelease( theString2 );
  return result;
#else
   return rhs.compare(lhs);
#endif
}


double UT_W2Float( const UT_String &s )
{
  string	as;
  for ( UT_String::const_iterator it = s.begin(); it != s.end(); ++it )
  {
    if ( *it == (UniChar_t)',' )
      as.insert( as.end(), '.' );
    else
      as.insert( as.end(), (char)*it );
  }
  return atof( as.c_str() );
}

int UT_W2Int( const UT_String &s )
{
  string	as;
  for ( UT_String::const_iterator it = s.begin(); it != s.end(); ++it )
  {
    as.insert( as.end(), (char)*it );
  }
  return atoi( as.c_str() );
}

