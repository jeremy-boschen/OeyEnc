/*TABS=3,SPACES=YES,LINE=128*/

/* OeyEnc - Outlook Express plugin for decoding messages encoded with
 * the yEnc format. 
 *
 * Copyright (C) 2004-2008 Jeremy Boschen. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software. 
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in
 * a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must not
 * be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

/*  yDecode.h
 *      yEnc V1.3 decoding implementation
 */

#pragma once

#ifndef __YDECODE_H__
#define __YDECODE_H__

#include <sal.h>

//#pragma strict_gs_check(push)
#pragma strict_gs_check(on)

#ifdef __cplusplus
namespace yEnc
{
#endif /* __cplusplus */

/**
 * Tags and attributes. Note that spaces are included.
 */
#define MK_YBEGIN             "=ybegin "
#define MK_YBEGIN2            "=ybegin2 "
#define MK_YPART              "=ypart "
#define MK_YEND               "=yend "
#define MK_LINE               " line="
#define MK_SIZE               " size="
#define MK_NAME               " name="
#define MK_PART               " part="
#define MK_TOTAL              " total="
#define MK_OFFSET             " offset="
#define MK_ESCAPE             " escape="
#define MK_ESCOFF             " escoff="
#define MK_COMP               " comp="
#define MK_BEGIN              " begin="
#define MK_END                " end="
#define MK_PCRC32             " pcrc32="
#define MK_CRC32              " crc32="

/* Tags without the required spaces */
#define MK_RAW_YBEGIN         "=ybegin"
#define MK_RAW_YBEGIN2        "=ybegin2"
#define MK_RAW_YPART          "=ypart"
#define MK_RAW_YEND           "=yend"
#define MK_RAW_LINE           "line="
#define MK_RAW_SIZE           "size="
#define MK_RAW_NAME           "name="
#define MK_RAW_PART           "part="
#define MK_RAW_TOTAL          "total="
#define MK_RAW_OFFSET         "offset="
#define MK_RAW_ESCAPE         "escape="
#define MK_RAW_ESCOFF         "escoff="
#define MK_RAW_COMP           "comp="
#define MK_RAW_BEGIN          "begin="
#define MK_RAW_END            "end="
#define MK_RAW_PCRC32         "pcrc32="
#define MK_RAW_CRC32          "crc32="

/* Miscellaneous tags */
#define MK_CRLF               "\r\n"
#define MK_CRLFCRLF           "\r\n\r\n"
#define MK_YENC               "yEnc"
#define MK_UNKNOWN            "Unknown"

/**
 * Suffixes
 */
#define MK_BROKEN    ".broken"

/*
 * Special values
 */
#define MV_ESCAPE    '='
#define MV_DEFAULT   '*'
#define MV_OFFSET    42
#define MV_ESCOFF    64
#define MV_LINEMIN   63
#define MV_LINEMAX   998
#define MV_SIZEMIN   1
#define MV_SIZEMAX   UINT_MAX
#define MV_PARTMIN   1
#define MV_PARTMAX   UINT_MAX
#define MV_TOTALMIN  1
#define MV_TOTALMAX  UINT_MAX
#define MV_OFFSETMIN 0
#define MV_OFFSETMAX 255
#define MV_ESCAPEMIN 0
#define MV_ESCAPEMAX 255
#define MV_ESCOFFMIN 1
#define MV_ESCOFFMAX 255
#define MV_BEGINMIN  1
#define MV_BEGINMAX  (UINT_MAX - 1)
#define MV_ENDMIN    2
#define MV_ENDMAX    UINT_MAX
#define MV_HEXDIGITS 8
#define MV_DECDIGITS 10
#define MV_MAXLINE   (sizeof(MK_LINE)   + sizeof(MK_SIZE)   + sizeof(MK_NAME)   + \
                      sizeof(MK_PART)   + sizeof(MK_TOTAL)  + sizeof(MK_OFFSET) + \
                      sizeof(MK_ESCAPE) + sizeof(MK_ESCOFF) + sizeof(MK_COMP)   + \
                      sizeof(MK_BEGIN)  + sizeof(MK_END)    + sizeof(MK_PCRC32) + \
                      sizeof(MK_CRC32)  + MAX_PATH)

/**
 * Critical characters
 */
#define MC_CR        '\r'
#define MC_LF        '\n'
#define MC_NUL       '\0'
#define MC_TAB       '\t'
#define MC_SPACE     ' '
#define MC_DQUOTE    '"'
#define MC_SQUOTE    '\''
#define MC_DOT       '.'
#define MC_ESCAPE    '='
#define MC_YCHAR     'y'
#define MC_2CHAR     '2'
#define MC_FSLASH    '\\'
#define MC_BSLASH    '/'

/**********************************************************************

    Utility

 **********************************************************************/

#ifndef _charsof
   #define _charsof( sz )  ((sizeof(sz) / sizeof(sz[0])) - 1)
#endif

#ifndef _countof
   #define _countof( rg )  (sizeof(rg) / sizeof(rg[0]))
#endif

#ifndef FlagOn
   #define FlagOn(v, f)    (0 != ((v) & (f)))
#endif

#ifndef FlagOff
   #define FlagOff(v, f)   (0 == ((v) & (f)))
#endif

#ifndef _ltext
   #define __ltext(x)      L ## x
   #define _ltext( x )     __ltext(x)
#endif

typedef const BYTE* LPCBYTE;

/**
 * CRC32
 */
typedef unsigned __int32 CRC32;

typedef struct _CRC32TABLE
{
   CRC32 Val[256];
}CRC32TABLE, *PCRC32TABLE;

typedef const CRC32TABLE* PCCRC32TABLE;

#ifdef _STATICCRCTABLE
/**
 * __CRC32TABLE
 *    Static table of CRC32 values for all 255 values of a byte
 */
static const CRC32TABLE __CRC32TABLE =
{
   0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
#else /* _STATICCRCTABLE */
/**
 * BuildCrc32Table
 */
static void __stdcall BuildCrc32Table( PCRC32TABLE pCrc32Table )
{
   /* Reversed 0x04C11DB7 */
   enum { ePolynomial = 0xEDB88320 };

   C_ASSERT(UCHAR_MAX >= (_countof(pCrc32Table->Val) - 1));

   for ( unsigned short i = 0; i < _countof(pCrc32Table->Val); i++ )
   {
      unsigned __int32 iCrc = i;

      for ( unsigned char j = 8; j > 0; j-- )
      {
         if ( 1 == (iCrc & 1) )
         {
            iCrc = (iCrc >> 1) ^ ePolynomial;
         }
         else
         {
            iCrc >>= 1;
         }
      }

      pCrc32Table->Val[i] = iCrc;
   }
}
#endif /* _STATICCRCTABLE */

/**
 * CalculateChecksum
 *    Calculates the CRC32 value for a block of bytes
 */
static CRC32 __stdcall CalculateChecksum( PCCRC32TABLE pCrc32Table, LPCBYTE pBuff, size_t cbBuf, CRC32 crc32 = 0xffffffff )
{
   while ( cbBuf-- )
   {
      crc32 = ((crc32 >> 8) ^ pCrc32Table->Val[(crc32 & 0x000000ff) ^ *pBuff++]);
   }

   return ( crc32 ^ 0xffffffff );
}

/**
 * FindTag
 *    Like strstr but length restricted.
 *
 * Parameters:
 *    [in] pbBuffer
 *       The input data to be searched
 *    [in] cbBuffer
 *       Size, in _bytes_, of the data pointed to by pbBuffer
 *    [in] pszTag
 *       The tag to search for
 *    [in] cbTag
 *       Size, in _bytes_, of the data pointed to by pszTag
 *    [in] bNewLine
 *       true to specify that pszTag must begin on a new line
 *
 * Return Value
 *    Returns the byte offset of the tag if found, NULL otherwise.
 */
static LPCBYTE __stdcall FindTag( __deref_in_bcount(cbBuffer) LPCBYTE pbBuffer, size_t cbBuffer, __deref_in_bcount(cbTag) LPCBYTE pszTag, size_t cbTag, bool bNewLine )
{
   _ASSERTE(NULL != pbBuffer);
   _ASSERTE(NULL != pszTag);

   /* If the tag to search for is greater than the available
    * buffer to search then its a no brainer */
   if ( cbBuffer < cbTag )
   {
      /* Failure */
      return ( NULL );
   }

   /* cbTag is a count, we want offsets */
   _ASSERTE(cbTag > 0);
   cbTag--;

   /* idx is the current source offset, jdx is the current tag
    * offset. When jdx == cbTag then we've found the tag within
    * the source buffer. */
   for ( size_t idx = 0, jdx = 0; idx < cbBuffer; idx++ )
   {
      if ( pbBuffer[idx] == pszTag[jdx] )
      {
         if ( jdx == cbTag )
         {
            /* If the caller doesn't require a new line, succeed. Otherwise if
             * there are two characters preceeding the tag in the source buffer
             * and they are equal to \r\n, succeed. A new line is considered
             * either the start of the source buffer, or a new line. So either
             * of the following are supported when bNewLine is true:
             *
             *    offset: 0 1 2 3 4 5 6 7 8 9        
             *    data:   t a g _ x y z . . .
             *
             *    offset: 0 1 2 3 4 5 6 7 8 9
             *    data:   \r\nt a g _ x y z .
             */
            if ( !bNewLine )
            {
               /* Success */
               return ( &pbBuffer[idx - cbTag] );
            }
            else
            {
               if ( idx == cbTag )
               {
                  /* Success */
                  return ( &pbBuffer[idx - cbTag] );
               }

               if ( (idx - cbTag) >= (2 * sizeof(BYTE)) )
               {
                  BYTE chLf = pbBuffer[idx - cbTag - 1];
                  BYTE chCr = pbBuffer[idx - cbTag - 2];

                  if ( (MC_LF == chLf) && (MC_CR == chCr) )
                  {
                     /* Success */
                     return ( &pbBuffer[idx - cbTag] );
                  }
               }
            }

            /* If this point is reached, the tag is required to be on a
             * new line but wasn't, so reset the search */
            jdx = 0;
         }
         else
         {
            jdx++;
         }
      }
      else
      {
         jdx = 0;
      }
   }

   /* Failure */
   return ( NULL );
}

/**
 * FindLineTag
 *    Wraps FindTag w/ bNewLine == true. Also, last character of
 *    'tag' is assumed to be a null-term and is ignored.
 */
#define FindLineTag( buf, cbbuf, tag ) \
   FindTag(buf, cbbuf, (LPCBYTE)tag, (sizeof(tag) - sizeof(tag[0])), true)

/**
 * FindAttributeTag
 *    Wraps FindTag w/ bNewLine == false. Also, last character of
 *    'tag' is assumed to be a null-term and is ignored.
 */
#define FindAttributeTag( buf, cbbuf, tag ) \
   FindTag(buf, cbbuf, (LPCBYTE)tag, (sizeof(tag) - sizeof(tag[0])), false)

/**
 * FindNextLine
 *    Locates the start of the next line.
 *
 * Return Value
 *    Returns the address following a new line, or NULL if a new line cannot
 *    be found.
 *
 * Remarks
 *    A line is \r\n (CRLF)
 */
inline LPCBYTE __stdcall FindNextLine( __deref_in_bcount(cbBuffer) LPCBYTE pbBuffer, size_t cbBuffer )
{
   _ASSERTE(NULL != pbBuffer);
   _ASSERTE(cbBuffer > 0);

   LPCBYTE pbLine = FindTag(pbBuffer, cbBuffer, (LPCBYTE)MK_CRLF, _charsof(MK_CRLF), false);
   if ( NULL != pbLine )
   {
      pbLine += _charsof(MK_CRLF);
   }

   /* Success / Failure */
   return ( pbLine );
}

/**
 * StrToUint
 *    Like atol but unsigned and length checked 
 */
static UINT __stdcall StrToUint( __deref_in_ecount(cchsz) LPCSTR psz, size_t cchsz, UINT iBase )
{
   UINT cValue  = 0;
   bool bUseHex = (16 == iBase ? true : false);

   _ASSERTE(NULL != psz);

   while ( cchsz > 0 )
   {
      if ( (*psz >= '0') && (*psz <= '9') )
      {
         cValue = ((cValue * iBase) + (0x0f & (*psz - '0')));
      }
      else if ( bUseHex && (*psz >= 'A') && (*psz <= 'F') )
      {
         cValue = ((cValue * iBase) + (0x0f & (*psz - 'A' + 10)));
      }
      else if ( bUseHex && (*psz >= 'a') && (*psz <= 'f') )
      {
         cValue = ((cValue * iBase) + (0x0f & (*psz - 'a' + 10)));
      }
      else
      {
         break;
      }

      cchsz--;
      psz++;
   }

   return ( cValue );
}

/**********************************************************************

    yEnc

 **********************************************************************/

/**
 * ENCODEINFO
 *    Data passed into and out of DecoderXxx functions for tracking
 *    information related to yEnc data
 */
typedef struct _ENCODEINFO 
{
   DWORD       EncodeFlags;
   USHORT      LineEncoded;
   size_t      SizeActual;
   size_t      SizeEncoded;
   size_t      BeginEncoded;
   size_t      EndEncoded;
   BYTE        OffsetEncoded;
   BYTE        EscapeEncoded;
   BYTE        EscOffEncoded;
   USHORT      PartEncoded;
   USHORT      TotalEncoded;
   CRC32       CrcEncoded;
   CRC32       PartCrcEncoded;   
   LPCBYTE     CompEncoded;
   USHORT      CompEncodedLength;   
   LPCBYTE     NameEncoded;
   USHORT      NameEncodedLength;
   LPCBYTE     DataStart;
}ENCODEINFO, *PENCODEINFO;

typedef ENCODEINFO const * PCENCODEINFO;

/* Mask of which members are valid */
#define EIF_LINE_ENCODED      0x00000001
#define EIF_SIZE_ACTUAL       0x00000002
#define EIF_SIZE_ENCODED      0x00000004
#define EIF_NAME_ENCODED      0x00000008
#define EIF_COMP_ENCODED      0x00000010
#define EIF_OFFSET_ENCODED    0x00000020
#define EIF_ESCAPE_ENCODED    0x00000040
#define EIF_ESCOFF_ENCODED    0x00000080
#define EIF_BEGIN_ENCODED     0x00000100
#define EIF_END_ENCODED       0x00000200
#define EIF_PART_ENCODED      0x00000400
#define EIF_TOTAL_ENCODED     0x00000800
#define EIF_CRC_ENCODED       0x00001000
#define EIF_PARTCRC_ENCODED   0x00002000

/* Other miscellaneous bits */
#define EIF_UNICODE_FILENAME  0x00100000
#define EIF_YENCVERSION1      0x01000000
#define EIF_YENCVERSION2      0x02000000

#define GetyEncVersion( dw )  ((0x0f000000 & dw) >> 24)

/* Error codes set by DecoderXxx functions */
#define YDEC_E_MISSING_TAG    0x8CE0A001
#define YDEC_E_INVALID_TAG    0x8CE0A002
#define YDEC_E_INVALID_VALUE  0x8CE0A003
#define YDEC_E_INVALID_ESCAPE 0x8CE0A004
#define YDEC_E_NO_MORE_DATA   0x8CE0A005
#define YDEC_E_INVALID_DATA   0x8CE0A006 

/**
 * DecoderFindEncodedBody
 *    Locates the start and end of a yEnc encoded block of text.
 *
 * Parameters:
 *    pbBuffer
 *       [in]  The text/byte buffer to scan
 *    cbBuffer
 *       [in] The size in bytes of the buffer
 *    ppbTail
 *       [out] Optional pointer to receive the end of the yEnc 
 *       encoded block. Note that this is the byte following the
 *       buffer pointed to by pbBuffer and not the last byte in
 *       the buffer.
 *
 * Returns:
 *    A pointer within the bounds of (pbBuffer+cbBuffer) if successful,
 *    otherwise NULL.
 *
 * Remarks:
 *    This function can be called repeatedly to find multiple yEnc 
 *    encoded bodies by passing the value returned in ppbTail as
 *    the pbBuffer parameter in the next call. When the function
 *    returns NULL, there are no more yEnc encoded bodies. However,
 *    be sure to also update cbBuffer to reflect the reduction in
 *    remaining buffer size for each successive call.
 */
static LPCBYTE __stdcall DecoderFindEncodedBody( __deref_in_bcount(cbBuffer) LPCBYTE pbBuffer, size_t cbBuffer, __out_opt LPCBYTE* ppbTail )
{
   _ASSERTE(NULL != pbBuffer);
   _ASSERTE(cbBuffer > 0);

   LPCBYTE pbBegin = NULL;
   LPCBYTE pbEnd   = NULL;
   LPCBYTE pbTail  = NULL;
   
   if ( NULL != ppbTail )
   {
      *ppbTail = NULL;
   }

   /* Find the first =ybegin[2] */
   pbBegin = FindLineTag(pbBuffer, cbBuffer, MK_YBEGIN2);
   if ( !pbBegin )
   {
      pbBegin = FindLineTag(pbBuffer, cbBuffer, MK_YBEGIN);
   }

   if ( !pbBegin )
   {
      /* Failure */
      return ( NULL );
   }

   if ( !ppbTail )
   {
      /* Success */
      return ( pbBegin );
   }

   /* Find the =yend line. If that fails, pbTail remains NULL */
   pbEnd = FindLineTag(pbBegin, cbBuffer - (pbBegin - pbBuffer), MK_YEND);
   if ( NULL != pbEnd )
   {
      /* Find the tail of the =yend line, if any */
      pbTail = FindNextLine(pbEnd, cbBuffer - (pbEnd - pbBuffer));
      if ( !pbTail )
      {
         /* The tail is the end of the buffer */
         pbTail = pbBuffer + cbBuffer;
      }
   }

   (*ppbTail) = pbTail;

   /* Success / Failure */
   return ( pbBegin );
}

/**
 * DecoderMakeFileName
 *    Builds a file name from parameters resulting from decoding
 *    a yEnc buffer
 *
 * Parameters:
 *    pwszBuf
 *       [out] Buffer to receive filename
 *    cchBuf
 *       [in] Size of buffer in characters
 *    uCodePage
 *       [in] Code page used when performing conversions
 *    pbName
 *       [in] The encoded name attribute
 *    cbName
 *       [in] Size of name in bytes
 *    pszComp
 *       [in] The encoded compression tag or NULL to omit
 *    cbComp
 *       [in] The size of the compression tag in bytes
 *    iPart
 *       [in] The part number or 0 to omit
 *    iBadCrc
 *       [in] The encoded CRC32 value which does not match the
 *       the computed CRC32 value, or 0 to omit
 *    cbBadSize
 *       [in] The encoded size value which does not match the
 *       computed size value, or 0 to omit
 *    pcchBaseName
 *       [out] Optional size in characters of the base filename
 *
 * Return Value
 *    Returns TRUE if successful, FALSE otherwise. On failure,
 *    the contents of pwszBuf are not destroyed.
 *
 * Remarks
 *    The resulting filename will have the following format:
 *       name[(crc-HEX[ ]size-UINT)].ext[.comp][.part]
 */
static BOOL __stdcall DecoderMakeFileName( __out_ecount(cchBuf) LPWSTR pwszBuf, size_t cchBuf, UINT uCodePage, __deref_in_bcount(cbName) LPCBYTE pbName, USHORT cbName, __deref_in_bcount(cbComp) LPCBYTE pbComp, USHORT cbComp, USHORT iPart, CRC32 iBadCrc, size_t cbBadSize, __out_opt size_t* pcchBaseName )
{
   bool    bUnicode = false;
   LPCBYTE pbExt    = NULL;

   if ( NULL != pcchBaseName )
   {
      (*pcchBaseName) = cchBuf;
   }

   if ( IsTextUnicode(pbName, (int)cbName, NULL) )
   {
      /* Find the last L'.' in the name data */
      LPCBYTE pbTmp = pbName;
      while ( true )
      {
         pbTmp = (LPCBYTE)FindAttributeTag(pbTmp, cbName - (pbTmp - pbName), L".");
         if (!pbTmp )
         {
            break;
         }
         pbExt = pbTmp++;
      }

      if ( NULL != pbExt )
      {
         /* If the name really is UNICODE, then the L'.' found will have to be a
          * multiple of sizeof(WCHAR) */
         if ( 0 == ((pbExt - pbName) % sizeof(WCHAR)) )
         {
            bUnicode = true;
         }
      }
      else 
      {
         bUnicode = true;
      }
   }

   if ( !bUnicode )
   {
      /* Find the last '.' in the name data */
      LPCBYTE pbTmp = pbName;
      pbExt = NULL;
      while ( true )
      {
         pbTmp = (LPCBYTE)FindAttributeTag(pbTmp, cbName - (pbTmp - pbName), ".");
         if ( !pbTmp )
         {
            break;
         }
         pbExt = pbTmp++;
      }
   }

   if ( bUnicode )
   {
      if ( FAILED(StringCchCopyNExW(pwszBuf, 
                                    cchBuf, 
                                    (const wchar_t*)pbName, 
                                    (NULL != pbExt ? pbExt - pbName : cbName) / sizeof(WCHAR), 
                                    &pwszBuf, 
                                    &cchBuf, 
                                    0)) )
      {
         /* Failure */
         return ( FALSE );
      }
   }
   else
   {
      int iCopied = MultiByteToWideChar(uCodePage, 
                                        0, 
                                        (LPCSTR)pbName, 
                                        (int)(NULL != pbExt ? pbExt - pbName : cbName), 
                                        pwszBuf, 
                                        (int)cchBuf);

      if ( 0 == iCopied )
      {
         /* Failure */
         return ( FALSE );
      }

      pwszBuf += iCopied;
      cchBuf  -= iCopied;
   }

   if ( (0 != iBadCrc) || (0 != cbBadSize) )
   {
      if ( 0 == cchBuf )
      {
         /* Failure */
         return ( FALSE );
      }

      size_t cbArg1  = (size_t)iBadCrc;
      size_t cbArg2  = (size_t)cbBadSize;
      LPWSTR pwszArg = L"(crc-%x)";

      if ( (0 != iBadCrc) && (0 != cbBadSize) )
      {
         pwszArg = L"(crc-%x size-%u)";
      }
      else if ( 0 != cbBadSize )
      {
         pwszArg = L"(size-%u)";
         cbArg1 = cbBadSize;
      }

      if ( FAILED(StringCchPrintfExW(pwszBuf, 
                                     cchBuf, 
                                     &pwszArg, 
                                     NULL, 
                                     0, 
                                     pwszArg, 
                                     cbArg1, 
                                     cbArg2)) )
      {
         /* Failure */
         return ( FALSE );
      }

      cchBuf -= (pwszArg - pwszBuf);
      pwszBuf = pwszArg;
   }

   /* Tack on the extension now */
   if ( NULL != pbExt )
   {
      if ( 0 == cchBuf )
      {
         /* Failure */
         return ( FALSE );
      }

      if ( bUnicode )
      {
         if ( FAILED(StringCchCopyNExW(pwszBuf, 
                                       cchBuf, 
                                       (const wchar_t*)pbExt, 
                                       (pbName + cbName - pbExt) / sizeof(WCHAR), 
                                       &pwszBuf, 
                                       &cchBuf, 
                                       0)) )
         {
            /* Failure */
            return ( FALSE );
         }
      }
      else
      {
         int iCopied = MultiByteToWideChar(uCodePage, 
                                           0, 
                                           (LPCSTR)pbExt, 
                                           (int)(pbName + cbName - pbExt), 
                                           pwszBuf, 
                                           (int)cchBuf);
         if ( 0 == iCopied )
         {
            /* Failure */
            return ( FALSE );
         }

         pwszBuf += iCopied;
         cchBuf  -= iCopied;
      }
   }

   /* Tack on the compression ext */
   if ( NULL != pbComp )
   {
      if ( cchBuf < 2 )
      {
         /* Failure */
         return ( FALSE );
      }

      *pwszBuf = L'.';

      pwszBuf++;
      cchBuf--;

      int iCopied = MultiByteToWideChar(uCodePage, 
                                        0, 
                                        (LPCSTR)pbComp, 
                                        (int)cbComp, 
                                        pwszBuf, 
                                        (int)cchBuf);
      if ( 0 == iCopied )
      {
         /* Failure */
         return ( FALSE );
      }

      pwszBuf += iCopied;
      cchBuf  -= iCopied;
   }

   if ( NULL != pcchBaseName )
   {
      (*pcchBaseName) -= cchBuf;
   }
   
   /* Add the part number */
   if ( 0 != iPart )
   {
      if ( (0 == cchBuf) || FAILED(StringCchPrintfExW(pwszBuf, 
                                                      cchBuf, 
                                                      &pwszBuf, 
                                                      &cchBuf, 
                                                      0, 
                                                      L".%02u", 
                                                      iPart)) )
      {
         /* Failure */
         return ( FALSE );
      }
   }

   if ( 0 == cchBuf )
   {
      /* Failure */
      return ( FALSE );
   }

   *pwszBuf = L'\0';

   /* Success */
   return ( TRUE );
}

/**
 * DecoderGetPartSize
 *    Utility function to return the size specified in begin/end
 *    if present, otherwise to return the default size
 */
__inline size_t DecoderGetPartSize( __deref_in PCENCODEINFO pei )
{
   if ( FlagOn(pei->EncodeFlags, EIF_BEGIN_ENCODED) && 
        FlagOn(pei->EncodeFlags, EIF_END_ENCODED) )
   {
      return ( (pei->EndEncoded - pei->BeginEncoded) + 1 );
   }

   return ( FlagOn(pei->EncodeFlags, EIF_SIZE_ENCODED) ? pei->SizeEncoded : 0 );
}

/**
 * DecoderGetFileName
 *    Utility function to wrap DecoderMakeFileName
 */
static BOOL DecoderGetFileName( __out_ecount(cchBuf) LPWSTR pszBuf, size_t cchBuf, UINT uCodePage, __deref_in PCENCODEINFO pei, __deref_in LPCBYTE pbDecoded, __out_opt size_t* pcchBaseName, __out_opt CRC32* piCrcCalculated, __deref_opt_in PCCRC32TABLE pCrc32Table )
{
   USHORT iPart;
   CRC32  iCrc32;
   size_t cbSize;
   
   iCrc32 = 0;
   cbSize = DecoderGetPartSize(pei);

   if ( (NULL != pCrc32Table) && (NULL != pbDecoded) )
   {
      if ( FlagOn(pei->EncodeFlags, EIF_PARTCRC_ENCODED) )
      {
         iCrc32 = CalculateChecksum(pCrc32Table, pbDecoded, pei->SizeActual, (ULONG)-1);
         if ( NULL != piCrcCalculated )
         {
            (*piCrcCalculated) = iCrc32;
         }

         if ( iCrc32 == pei->PartCrcEncoded )
         {
            iCrc32 = 0;
         }
      }
      else if ( FlagOn(pei->EncodeFlags, EIF_CRC_ENCODED) )
      {
         iCrc32 = CalculateChecksum(pCrc32Table, pbDecoded, pei->SizeActual, (ULONG)-1);
         if ( NULL != piCrcCalculated )
         {
            (*piCrcCalculated) = iCrc32;
         }

         if ( iCrc32 == pei->CrcEncoded )
         {
            iCrc32 = 0;
         }
      }     
   }

   if ( cbSize == pei->SizeActual )
   {
      cbSize = 0;
   }

   /* Parts without totals and the first part of a set are always given a name without the part number. */
   iPart = ((pei->PartEncoded  > 1) && 
            (pei->TotalEncoded > 1) &&
            FlagOn(pei->EncodeFlags, EIF_PART_ENCODED|EIF_TOTAL_ENCODED) ? pei->PartEncoded : 0);

   return ( DecoderMakeFileName(pszBuf, 
                                cchBuf, 
                                uCodePage, 
                                FlagOn(pei->EncodeFlags, EIF_NAME_ENCODED) ? pei->NameEncoded       : (LPCBYTE)MK_UNKNOWN,
                                FlagOn(pei->EncodeFlags, EIF_NAME_ENCODED) ? pei->NameEncodedLength : _charsof(MK_UNKNOWN),
                                FlagOn(pei->EncodeFlags, EIF_COMP_ENCODED) ? pei->CompEncoded       : NULL,
                                FlagOn(pei->EncodeFlags, EIF_COMP_ENCODED) ? pei->CompEncodedLength : 0,
                                iPart,
                                iCrc32,
                                cbSize,
                                pcchBaseName) );
}

/**
 * DecoderSerializeEncodeInfo
 *    Utility function to build a string representation of an ENCODEINFO 
 *    record. The resulting string can later be converted back into the
 *    same ENCODEINFO record with the DecoderDeserializeEncodeInfo function.
 */
static BOOL __stdcall DecoderSerializeEncodeInfo( __out_ecount(cchBuf) LPWSTR pszValue, size_t cchValue, UINT uCodePage, DWORD dwFlags, __deref_in PCENCODEINFO pei )
{
   _ASSERTE(NULL != pszValue && cchValue > 0);
   _ASSERTE(NULL != pei);

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_LINE_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_LINE) L"%u", 
                                     pei->LineEncoded)) )
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_SIZE_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_SIZE) L"%u", 
                                     pei->SizeEncoded)) )
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_COMP_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_COMP) L"%.*s", 
                                     pei->CompEncodedLength, 
                                     pei->CompEncoded) ) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_OFFSET_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_OFFSET) L"%u", 
                                     pei->OffsetEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_ESCAPE_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_ESCAPE) L"%u", 
                                     pei->EscapeEncoded)) )
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_ESCOFF_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_ESCOFF) L"%u", 
                                     pei->EscOffEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_BEGIN_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_BEGIN) L"%u", 
                                     pei->BeginEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_END_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_END) L"%u", 
                                     pei->EndEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_PART_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_PART) L"%u", 
                                     pei->PartEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_TOTAL_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_TOTAL) L"%u", 
                                     pei->TotalEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_CRC_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_CRC32) L"%0.8lx", 
                                     pei->CrcEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_PARTCRC_ENCODED) )
   {
      if ( FAILED(StringCchPrintfExW(pszValue, 
                                     cchValue, 
                                     &pszValue, 
                                     &cchValue, 
                                     0, 
                                     _ltext(" ") _ltext(MK_RAW_PCRC32) L"%0.8lx", 
                                     pei->PartCrcEncoded)) ) 
      {
         return ( FALSE );
      }
   }

   if ( FlagOn(dwFlags & pei->EncodeFlags, EIF_NAME_ENCODED) )
   {
      WCHAR chFile[MAX_PATH] = {L'\0'};

      if ( !DecoderMakeFileName(chFile, 
                               _countof(chFile), 
                               uCodePage, 
                               pei->NameEncoded, 
                               pei->NameEncodedLength, 
                               pei->CompEncoded, 
                               pei->CompEncodedLength, 
                               0, 
                               0, 
                               0, 
                               NULL) )
      {
         return ( FALSE );
      }

      if ( FAILED(StringCchPrintfExW(pszValue, 
                  cchValue, 
                  &pszValue, 
                  &cchValue, 
                  0, 
                  _ltext(" ") _ltext(MK_RAW_NAME) L"\"%s\"", 
                  chFile)) )
      {
         return ( FALSE );
      }
   }

   return ( TRUE );
}

/**
 * DecoderDeserializeEncodeInfo
 *    Parses a message header encoded by DecoderSerializeEncodeInfo and
 *    builds a matching ENCODEINFO structure
 */
static void __stdcall DecoderDeserializeEncodeInfo( __in_bcount_opt(cbValue) LPCSTR pszValue, size_t cbValue, DWORD dwFlags, __deref_inout PENCODEINFO pei )
{
   LPCBYTE pbTag;

/*TAG: line= */
   if ( FlagOn(dwFlags, EIF_LINE_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_LINE);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_LINE);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_LINE_ENCODED;
         pei->LineEncoded  = (USHORT)(0xffff & iValue);
      }
   }

/*TAG: size= */
   if ( FlagOn(dwFlags, EIF_SIZE_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_SIZE);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_SIZE);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_SIZE_ENCODED;
         pei->SizeEncoded  = iValue;
      }
   }
   
/*TAG: begin= */
   if ( FlagOn(dwFlags, EIF_BEGIN_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_BEGIN);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_BEGIN);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_BEGIN_ENCODED;
         pei->BeginEncoded = iValue;
      }
   }

/*TAG: end= */
   if ( FlagOn(dwFlags, EIF_END_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_END);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_END);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_END_ENCODED;
         pei->EndEncoded   = iValue;
      }
   }
   
/*TAG: offset= */
   if ( FlagOn(dwFlags, EIF_OFFSET_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_OFFSET);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_OFFSET);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags  |= EIF_OFFSET_ENCODED;
         pei->OffsetEncoded = (BYTE)(0xff & iValue);
      }
   }
   
/*TAG: escape= */
   if ( FlagOn(dwFlags, EIF_ESCAPE_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_ESCAPE);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_ESCAPE);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_ESCAPE_ENCODED;
         pei->EscapeEncoded = (BYTE)(0xff & iValue);
      }
   }

/*TAG: escoff= */
   if ( FlagOn(dwFlags, EIF_ESCOFF_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_ESCOFF);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_ESCOFF);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags  |= EIF_ESCOFF_ENCODED;
         pei->EscOffEncoded = (BYTE)(0xff & iValue);
      }
   }
   
/*TAG: part= */
   if ( FlagOn(dwFlags, EIF_PART_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_PART);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_PART);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_PART_ENCODED;
         pei->PartEncoded  = (USHORT)(0xffff & iValue);
      }
   }

/*TAG: total= */
   if ( FlagOn(dwFlags, EIF_TOTAL_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_TOTAL);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_TOTAL);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_DECDIGITS), 10);
         pei->EncodeFlags |= EIF_TOTAL_ENCODED;
         pei->TotalEncoded = (USHORT)(0xffff & iValue);
      }
   }

/*TAG: crc32= */
   if ( FlagOn(dwFlags, EIF_CRC_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_CRC32);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_CRC32);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_HEXDIGITS), 16);
         pei->EncodeFlags |= EIF_CRC_ENCODED;
         pei->CrcEncoded   = iValue;
      }
   }

/*TAG: pcrc32= */
   if ( FlagOn(dwFlags, EIF_PARTCRC_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_PCRC32);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_PCRC32);
         UINT iValue = StrToUint((LPCSTR)pbTag, min(cbValue - (pbTag - (LPCBYTE)pszValue), MV_HEXDIGITS), 16);
         pei->EncodeFlags   |= EIF_PARTCRC_ENCODED;
         pei->PartCrcEncoded = iValue;
      }
   }

/*TAG: comp= */
   if ( FlagOn(dwFlags, EIF_COMP_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_COMP);
      if ( NULL != pbTag )
      {
         pbTag            += _charsof(MK_RAW_COMP);
         pei->EncodeFlags |= EIF_COMP_ENCODED;
         pei->CompEncoded  = pbTag;

         size_t cchComp = 0; 
         while ( (cchComp < cbValue) && (MC_SPACE != *pbTag++) )
         {
            cchComp++;
         }

         pei->CompEncodedLength = (USHORT)cchComp;
      }
   }

/*TAG: name= */
   if ( FlagOn(dwFlags, EIF_NAME_ENCODED) )
   {
      pbTag = FindAttributeTag((LPCBYTE)pszValue, cbValue, MK_RAW_NAME);
      if ( NULL != pbTag )
      {
         pbTag += _charsof(MK_RAW_NAME);
         if ( MC_DQUOTE == *pbTag )
         {
            pbTag   += 1;
            cbValue -= 1;
         }
         
         pei->EncodeFlags |= EIF_NAME_ENCODED;      
         pei->NameEncoded  = pbTag;

         size_t cchName = 0;
         while ( (cchName < cbValue) && (MC_DQUOTE != *pbTag++) )
         {
            cchName++;
         }
         pei->NameEncodedLength = (USHORT)cchName;
      }  
   }
}

/**
 * DecoderGetSubjectParts
 *    Attempts to retrieve part and total information from a message subject
 */
static BOOL __stdcall DecoderGetSubjectParts( __in_bcount_nz(cbSubject) LPCSTR pszSubject, size_t cbSubject, __deref_inout PENCODEINFO pei )
{
   UINT iPart  = 0;
   UINT iTotal = 0;

   LPCSTR pszBuf = pszSubject;
   size_t cbBuf  = cbSubject;
   
   LPCSTR pszNum = pszBuf;

   while ( (MC_NUL != *pszBuf) && (cbBuf > 0) )
   {
      /* Prefer / to \ */
      if ( (MC_BSLASH == *pszBuf) || (MC_FSLASH == *pszBuf) )
      {
         if ( pszNum < pszBuf )
         {
            iPart  = StrToUint(pszNum, (size_t)(pszBuf - pszNum), 10);
            iTotal = StrToUint(pszBuf + 1, cbBuf - 1, 10);

            if ( (iPart  >= MV_PARTMIN)  && 
                 (iPart  <= MV_PARTMAX)  && 
                 (iTotal >= MV_TOTALMIN) && 
                 (iTotal <= MV_TOTALMAX) )
            {
               /* The last numbers in the subject line win */
               pei->EncodeFlags |= EIF_PART_ENCODED|EIF_TOTAL_ENCODED;
               pei->PartEncoded  = (USHORT)(0xffff & iPart);
               pei->TotalEncoded = (USHORT)(0xffff & iTotal);
            }
         }
      }
      else if ( (*pszBuf < '0') || (*pszBuf > '9') )
      {
         pszNum = pszBuf + 1;
      }

      pszBuf++;
      cbBuf--;
   }
   
   return ( FlagOn(pei->EncodeFlags, EIF_PART_ENCODED|EIF_TOTAL_ENCODED) ? TRUE : FALSE );
}

/**
 * DecoderInitialize
 *   Parses a yEnc header to initialize an ENCODEINFO record.
 *
 * Parameters:
 *    pbEncoded
 *       [in] Encoded buffer
 *    cbEncoded
 *       [in] Size in bytes of encoded buffer
 *    pEncInfo
 *       [out] Pointer to an ENCODEINFO structure to receive decoded
 *       initialization data.
 *
 * Returns:
 *    TRUE if successful, otherwise FALSE. Call GetLastError to determine
 *    the cause of failure.
 *
 * Remarks:
 *    This function assumes the input buffer has been properly validated
 *    and ignores characters preceeding the =ybegin[2] tag. Callers should
 *    use the DecoderFindEncodedBody function to validate the input buffer.
 *
 *    The implementation here is fairly simple, top-down. If new tags are
 *    added/modified to yEnc, code will have to be added/updated to take
 *    advantage of them.
 */
static BOOL __stdcall DecoderInitialize( __deref_in_bcount(cbEncoded) LPCBYTE pbEncoded, size_t cbEncoded, __deref_out PENCODEINFO pEncInfo )
{
   _ASSERTE(NULL != pbEncoded);
   _ASSERTE(cbEncoded > 0);
   _ASSERTE(NULL != pEncInfo);

   memset(pEncInfo, 0, sizeof(ENCODEINFO));

/**TAG: =ybegin | =ybegin2 (header) */
   LPCBYTE pbLine = NULL;
   LPCBYTE pbEnd  = pbEncoded + cbEncoded;

   if ( NULL != (pbLine = FindLineTag(pbEncoded, cbEncoded, MK_RAW_YBEGIN)) )
   {
      pbLine    += _charsof(MK_RAW_YBEGIN);
      cbEncoded -= _charsof(MK_RAW_YBEGIN);

      if ( cbEncoded > 0 )
      {
         if ( MC_SPACE == *pbLine )
         {
            pbLine    += 1;
            cbEncoded -= 1;

            pEncInfo->EncodeFlags |= EIF_YENCVERSION1;
         }
         else if ( MC_2CHAR == *pbLine )
         {
            pbLine    += 1;
            cbEncoded -= 1;

            if ( cbEncoded > 0 )
            {
               if ( MC_SPACE == *pbLine )
               {
                  pbLine    += 1;
                  cbEncoded -= 1;

                  pEncInfo->EncodeFlags |= EIF_YENCVERSION2;
               }
            }
         }
      }
   }

   if ( 0 == GetyEncVersion(pEncInfo->EncodeFlags) )
   {
      SetLastError(YDEC_E_MISSING_TAG);
      /* Failure - No yEnc header */
      return ( FALSE );
   }

   if ( 0 == cbEncoded )
   {
      SetLastError(YDEC_E_NO_MORE_DATA);
      /* Failure */
      return ( FALSE );
   }

   /* Move back 1 so 'line' has a space */
   pbLine--;
   cbEncoded++;
   
/**TAG: line= (header) */
   LPCBYTE pbTag = FindAttributeTag(pbLine, cbEncoded, MK_LINE);
   if ( !pbTag )
   {
      SetLastError(YDEC_E_MISSING_TAG);
      /* Failure */
      return ( FALSE );
   }

   pbTag += _charsof(MK_LINE);

   UINT cbLine = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
   cbLine &= USHRT_MAX;

   if ( !((cbLine >= MV_LINEMIN) && (cbLine <= MV_LINEMAX)) )
   {
      SetLastError(YDEC_E_INVALID_VALUE);
      /* Failure */
      return ( FALSE );
   }

   pEncInfo->EncodeFlags = EIF_LINE_ENCODED;
   pEncInfo->LineEncoded = (USHORT)cbLine;

/**TAG: size= (header) */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbTag), MK_SIZE);
   if ( !pbTag )
   {
      SetLastError(YDEC_E_MISSING_TAG);
      /* Failure */
      return ( FALSE );
   }

   pbTag += _charsof(MK_SIZE);

   size_t cbSizeHead = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
   if ( !((cbSizeHead >= MV_SIZEMIN) && (cbSizeHead <= MV_SIZEMAX)) )
   {
      SetLastError(YDEC_E_INVALID_VALUE);
      /* Failure */
      return ( FALSE );
   }

   pEncInfo->EncodeFlags |= EIF_SIZE_ENCODED;
   pEncInfo->SizeEncoded  = cbSizeHead;

   /* Get optional attributes that come before the filename */

/**TAG: part= (header) */
   UINT iPartHead = 0;
   pbTag = FindAttributeTag(pbLine, cbEncoded, MK_PART);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_PART);      
      
      iPartHead  = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      iPartHead &= USHRT_MAX;

      if ( !((iPartHead >= MV_PARTMIN) && (iPartHead <= MV_PARTMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags |= EIF_PART_ENCODED;
      pEncInfo->PartEncoded  = (USHORT)iPartHead;
   }

/**TAG: total= (header) */
   /* The 'total' attribute should be included if 'part' is present, but 
    * since it was added later it is optional. However if 'total' is present,
    * then 'part' is required. */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_TOTAL);
   if ( NULL != pbTag )
   {
      if ( 0 == iPartHead )
      {
         SetLastError(YDEC_E_MISSING_TAG);
         /* Failure */
         return ( FALSE );
      }

      pbTag  += _charsof(MK_TOTAL);
      
      UINT iTotal = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      iTotal &= USHRT_MAX;

      if ( !((iTotal >= MV_TOTALMIN) && (iTotal <= MV_TOTALMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags |= EIF_TOTAL_ENCODED;
      pEncInfo->TotalEncoded = (USHORT)iTotal;
   }

/**TAG: crc32= (header) */
   UINT iCrcHead = 0;

   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_CRC32);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_CRC32);

      /* Caller is responsible for CRC validation */
      iCrcHead = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_HEXDIGITS), 16);

      pEncInfo->EncodeFlags |= EIF_CRC_ENCODED;
      pEncInfo->CrcEncoded   = (CRC32)iCrcHead;
   }

/**TAG: offset= (header) */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_OFFSET);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_OFFSET);

      UINT bOffset = MV_OFFSET;

      /* First check if the '*' character was used */
      if ( MV_DEFAULT != *pbTag )
      {
         bOffset  = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
         bOffset &= UCHAR_MAX;

         #pragma warning( suppress : 4296 )
         if ( !((bOffset >= MV_OFFSETMIN) && (bOffset <= MV_OFFSETMAX)) )
         {
            SetLastError(YDEC_E_INVALID_VALUE);
            /* Failure */
            return ( FALSE );
         }
      }

      pEncInfo->EncodeFlags  |= EIF_OFFSET_ENCODED;
      pEncInfo->OffsetEncoded = (BYTE)bOffset;
   }

/**TAG: escape= (header) */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_ESCAPE);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_ESCAPE);

      UINT bEscape = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      bEscape &= UCHAR_MAX;

      #pragma warning( suppress : 4296 )
      if ( !((bEscape >= MV_ESCAPEMIN) && (bEscape <= MV_ESCAPEMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags  |= EIF_ESCAPE_ENCODED;
      pEncInfo->EscapeEncoded = (BYTE)bEscape;
   }

/**TAG: escoff= (header) */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_ESCOFF);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_ESCOFF);

      UINT bEscOff = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      bEscOff &= UCHAR_MAX;

      if ( !((bEscOff >= MV_ESCOFFMIN) && (bEscOff <= MV_ESCOFFMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags  |= EIF_ESCOFF_ENCODED;
      pEncInfo->EscOffEncoded = (BYTE)bEscOff;
   }

/**TAG: comp= (header) */
   /* Compression is limited to MAX_PATH characters and cannot have embedded spaces */
   size_t cch = min((pbEnd - pbLine), MAX_PATH);

   pbTag = FindAttributeTag(pbLine, cch, MK_COMP);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_COMP);
      
      LPCBYTE pbComp = pbTag;

      while ( cch > 0 )
      {
         if ( (MC_SPACE == *pbTag) || (MC_CR == *pbTag) || (MC_LF == *pbTag) )
         {
            break;
         }

         pbTag++;
         cch--;
      }

      pEncInfo->EncodeFlags      |= EIF_COMP_ENCODED;
      pEncInfo->CompEncoded       = pbComp;
      pEncInfo->CompEncodedLength = (USHORT)(pbTag - pbComp);
   }

/**TAG: name= (header) */
   pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_NAME);
   if ( !pbTag )
   {
      SetLastError(YDEC_E_MISSING_TAG);
      /* Failure */
      return ( FALSE );
   }

   pbTag     += _charsof(MK_NAME);
   cbEncoded -= (pbTag - pbLine);

   /* Skip the first character if it is a quote */
   if ( MC_DQUOTE == *pbTag )
   {
      pbTag++;
      cbEncoded--;
   }

   cch = min(cbEncoded, MAX_PATH);

   /* Run past any spaces */
   while ( cch > 0 )
   {
      if ( MC_SPACE != *pbTag )
      {
         break;
      }

      pbTag++;
      cch--;
   }
   
   LPCBYTE pbName = pbTag;
   UINT    iNull  = 0;

   /* The name attribute is required to be the last attribute for the header
    * line, so scan until the end of either the line or buffer. */
   while ( cch > 0 )
   {
      if ( (MC_CR == *pbTag) || (MC_LF == *pbTag) || (MC_DQUOTE == *pbTag) )
      {
         /* Backup one for the ' ' check... */
         pbTag--;
         cch++;
         break;
      }

      if ( MC_NUL == *pbTag )
      {
         iNull++;
      }
         
      pbTag++;
      cch--;
   }

   /* Remove any trailing spaces in the name */
   while ( pbTag > pbName )
   {
      if ( MC_SPACE != *pbTag )
      {
         /* Bump up one to include this character... */
         pbTag++;
         cch--;
         break;
      }

      pbTag--;
      cch++;
   }

   pEncInfo->EncodeFlags      |= EIF_NAME_ENCODED;
   pEncInfo->NameEncoded       = pbName;
   pEncInfo->NameEncodedLength = (USHORT)(pbTag - pbName);

   /* Also update the remaining buffer size */
   cbEncoded -= (pbTag - pbName);
   
   /* Set the unicode flag of NameLength if there were embedded nulls in the name and
    * its length is an even number */
   if ( (iNull > 1) && (0 == (pEncInfo->NameEncodedLength % 2)) )
   {
      pEncInfo->EncodeFlags |= EIF_UNICODE_FILENAME;
   }
   
   /* There shouldn't be anything after the name except a CRLF. Note that cbEncoded is
    * the size of the remaining buffer from pbTag, so we start the search from it */
   pbLine = FindNextLine(pbTag, (pbEnd - pbTag));
   if ( !pbLine )
   {
      SetLastError(YDEC_E_NO_MORE_DATA);
      /* Failure */
      return ( FALSE );
   }

   /* Update the remaining buffer size and make sure there is more to parse */
   cbEncoded -= (pbLine - pbTag);
   if ( 0 == cbEncoded )
   {
      SetLastError(YDEC_E_NO_MORE_DATA);
      /* Failure - Missing data */
      return ( FALSE );
   }

/**TAG: =ypart (info-line) */
   pbTag = FindLineTag(pbLine, (pbEnd - pbLine), MK_YPART);
   if ( NULL != pbTag )
   {
      if ( 0 == iPartHead )
      {
         SetLastError(YDEC_E_MISSING_TAG);
         /* Failure */
         return ( FALSE );
      }

      /* Make sure there was nothing between the =ybegin[2] and =ypart lines */
      if ( pbTag != pbLine )
      {
         SetLastError(YDEC_E_INVALID_TAG);
         /* Failure */
         return ( FALSE );
      }      

/**TAG: begin= (info-line) */
      pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_BEGIN);
      if ( !pbTag )
      {
         SetLastError(YDEC_E_MISSING_TAG);
         /* Failure */
         return ( FALSE );
      }

      pbTag += _charsof(MK_BEGIN);

      size_t cbBegin = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      if ( !((cbBegin >= MV_BEGINMIN) && (cbBegin <= MV_BEGINMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags |= EIF_BEGIN_ENCODED;
      pEncInfo->BeginEncoded = cbBegin;

/**TAG: end= (info-line) */
      pbTag = FindAttributeTag(pbLine, (pbEnd - pbLine), MK_END);
      if ( !pbTag )
      {
         SetLastError(YDEC_E_MISSING_TAG);
         /* Failure */
         return ( FALSE );
      }

      pbTag += _charsof(MK_END);

      size_t cbEnd = StrToUint((LPCSTR)pbTag, min((pbEnd - pbTag), MV_DECDIGITS), 10);
      if ( !((cbEnd >= MV_ENDMIN) && (cbEnd <= MV_ENDMAX)) )
      {
         SetLastError(YDEC_E_INVALID_VALUE);
         /* Failure */
         return ( FALSE );
      }

      pEncInfo->EncodeFlags |= EIF_END_ENCODED;
      pEncInfo->EndEncoded   = cbEnd;

      /* Update the line pointer and remaining buffer size so they point to the
       * start of the encoded data */
      pbLine     = FindNextLine(pbTag, (pbEnd - pbTag));
      cbEncoded -= (pbLine - pbTag);
   }

   if ( 0 == cbEncoded )
   {
      SetLastError(YDEC_E_NO_MORE_DATA);
      /* Failure */
      return ( FALSE );
   }

   pEncInfo->DataStart = pbLine;

   /* Success */
   return ( TRUE );
}

/**
 * DecoderProcessBuffer
 *    Decodes a block of data encoded in the yEnc format using
 *    settings initialized by a previous call to DecoderInitialize()
 *
 * Parameters
 *    pbEncoded
 *       [in] Pointer to the start of the yEnc data
 *    cbEncoded
 *       [in] Size of the data pointed to by pbEncoded, in bytes
 *    pEncInfo
 *       [in,out] Pointer to a previously intialized ENCODEINFO structure
 *    pbDecoded
 *       [out] Pointer to memory where the decoded bytes are written to
 *    cbDecoded
 *       [in] Size of the data pointed to by pbDecoded, in bytes
 *
 * Return Value
 *    Returns TRUE if successful, FALSE otherwise. Call GetLastError()
 *    to retrieve additional information when the function fails.
 *
 * Remarks
 *    None
 */
static BOOL __stdcall DecoderProcessBuffer( __deref_in_bcount(cbEncoded) LPCBYTE pbEncoded, __in size_t cbEncoded, __deref_inout PENCODEINFO pEncInfo, __deref_out_bcount(cbDecoded) PBYTE pbDecoded, __in size_t cbDecoded )
{
   _ASSERTE(NULL != pbEncoded && cbEncoded > 0);
   _ASSERTE(NULL != pEncInfo);
   _ASSERTE(NULL != pbDecoded && cbDecoded > 0);

   DWORD dwFlags   = pEncInfo->EncodeFlags;

   BYTE  bOffset   = FlagOn(dwFlags, EIF_OFFSET_ENCODED) ? pEncInfo->OffsetEncoded : MV_OFFSET;
   BYTE  bEscape   = FlagOn(dwFlags, EIF_ESCAPE_ENCODED) ? pEncInfo->EscapeEncoded : MV_ESCAPE;
   BYTE  bEscOff   = FlagOn(dwFlags, EIF_ESCOFF_ENCODED) ? pEncInfo->EscOffEncoded : MV_ESCOFF;

   size_t  cbBufIn    = cbEncoded;
   LPCBYTE pbBufIn    = pbEncoded;
   LPCBYTE pbBufInEnd = pbBufIn + cbBufIn;

   size_t cbBufOut = cbDecoded;
   PBYTE  pbBufOut = pbDecoded;

   /* The outer loop is for start of line processing */
   while ( cbBufIn && cbBufOut )
   {
/**TAG: =yend (trailer) */   
      if ( (cbBufIn < _charsof(MK_YEND)) || FindLineTag(pbBufIn, _charsof(MK_YEND), MK_YEND) )
      {
         /* Finished processing */
         break;
      }

      /* Make sure the line isn't starting with an invalid escape sequence, 
       * which would be anything but '=yend' */
      if ( (bEscape == *pbBufIn) && (MC_YCHAR == pbBufIn[1]) )
      {
         SetLastError(YDEC_E_INVALID_TAG);
         /* Failure */
         return ( FALSE );
      }

      /* The inner loop is for data decoding */
      while ( cbBufIn && cbBufOut )
      {
         /* Get a character from the input stream and update the pointer/count */
         BYTE bCh = *pbBufIn;
         pbBufIn += 1;
         cbBufIn -= 1;

         /* Test for a new line, which should probably be enforced as \r\n, but
          * currently any of \r\n, \r, \n are accepted. */
         if ( MC_CR == bCh )
         {
            if ( (cbBufIn > 0) && (MC_LF == *pbBufIn) )
            {
               pbBufIn += 1;
               cbBufIn -= 1;
            }

            /* Next line... */
            break;
         }
         else if ( MC_LF == bCh )
         {
            pbBufIn += 1;
            cbBufIn -= 1;

            /* Next line... */
            break;
         }

         if ( bEscape == bCh )
         {
            /* Escape characters cannot be the last character */
            if ( 0 == cbBufIn )
            {
               SetLastError(YDEC_E_INVALID_ESCAPE);
               /* Failure */
               return ( FALSE );
            }

            /* Get the escaped character and make sure it's valid */
            bCh      = *pbBufIn;            
            cbBufIn -= 1;
            pbBufIn += 1;

            /* Escape characters cannot be followed by line breaks */
            if ( (MC_CR == bCh) || (MC_LF == bCh) )
            {
               SetLastError(YDEC_E_INVALID_ESCAPE);
               /* Failure */
               return ( FALSE );
            }

            /* The escaped character is valid, so fix it up and continue */
            bCh = (bCh - bEscOff);
         }

         /* Decode the character, write it to the output stream and update the 
          * output stream pointer/count */
         *pbBufOut = (bCh - bOffset);
         pbBufOut += 1;
         cbBufOut -= 1;
      }/*END-OF: while ( cbBufIn && cbBufOut ) : inner-loop */
   }/*END-OF: while ( cbBufIn && cbBufOut ) : outer-loop*/

   pEncInfo->SizeActual = (cbDecoded - cbBufOut);
   
   /* If this point is reached then '=yend' was found or one of the buffers 
    * was exhausted. It doesn't matter if the output buffer was exhausted as 
    * nothing else needs to be writen to it */
   if ( 0 == cbBufIn )
   {
      SetLastError(YDEC_E_NO_MORE_DATA);
      /* Failure */
      return ( FALSE );
   }

/**TAG: size= (trailer) */
   LPCBYTE pbTag = FindAttributeTag(pbBufIn, (pbBufInEnd - pbBufIn), MK_SIZE);   
   if ( !pbTag )
   {
      SetLastError(YDEC_E_MISSING_TAG);
      /* Failure */
      return ( FALSE );
   }

   pbTag   += _charsof(MK_SIZE);
   cbBufIn -= _charsof(MK_SIZE);

#ifdef _DEBUG
   UINT cbSizeTail = StrToUint((LPCSTR)pbTag, min((pbBufInEnd - pbTag), MV_DECDIGITS), 10);   
   _ASSERTE(cbSizeTail == DecoderGetPartSize(pEncInfo));
#endif /* _DEBUG */

/**TAG: crc32= (trailer) */
   pbTag = FindAttributeTag(pbBufIn, cbBufIn, MK_CRC32);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_CRC32);

      UINT iCrcTail = StrToUint((LPCSTR)pbTag, min((pbBufInEnd - pbTag), MV_HEXDIGITS), 16);
      _ASSERTE(FlagOff(dwFlags, EIF_CRC_ENCODED) || iCrcTail == pEncInfo->CrcEncoded);      
      pEncInfo->EncodeFlags |= EIF_CRC_ENCODED;
      pEncInfo->CrcEncoded = (CRC32)iCrcTail;
   }

/**TAG: pcrc32= (trailer) */
   pbTag = FindAttributeTag(pbBufIn, cbBufIn, MK_PCRC32);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_PCRC32);

      UINT iCrcPart = StrToUint((LPCSTR)pbTag, min((pbBufInEnd - pbTag), MV_HEXDIGITS), 16);
      pEncInfo->EncodeFlags   |= EIF_PARTCRC_ENCODED;
      pEncInfo->PartCrcEncoded = (CRC32)iCrcPart;
   }

/**TAG: part= (trailer) */
   pbTag = FindAttributeTag(pbBufIn, cbBufIn, MK_PART);
   if ( NULL != pbTag )
   {
      pbTag += _charsof(MK_PART);

      UINT iPartTail = StrToUint((LPCSTR)pbTag, min((pbBufInEnd - pbTag), MV_HEXDIGITS), 10);
      _ASSERTE(FlagOff(dwFlags, EIF_PARTCRC_ENCODED) || pEncInfo->PartEncoded == iPartTail);
      pEncInfo->EncodeFlags |= EIF_PART_ENCODED;
      pEncInfo->PartEncoded  = (USHORT)(0xffff & iPartTail);
   }      

   /* Success */
   return ( TRUE );
}

#ifdef __cplusplus
} /* namespace yEnc */

using namespace yEnc;
#endif /* __cplusplus */

//#pragma strict_gs_check(pop)

#endif /* __YDECODE_H__ */