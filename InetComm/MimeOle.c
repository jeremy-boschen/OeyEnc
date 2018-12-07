
/* INETCOMM.LIB is missing from the latest WDK/WSK, so this project simply recreates it. 
 *
 * - Only those functions actually used by OeyEnc are listed 
 * - Functions provide no implementation, but its enough to get LINK and LIB to produce
 *   a useable import library.
 */

#define _MIMEOLE_
#include <mimeole.h>

MIMEOLEAPI MimeOleCreateMessage( /* in */ IUnknown* pUnkOuter, /* out */ IMimeMessage** ppMessage )
{
   pUnkOuter;
   ppMessage;
   return ( E_NOTIMPL );
}

MIMEOLEAPI MimeOleCreatePropertySet( /* in */ IUnknown* pUnkOuter, /* out */ IMimePropertySet** ppPropertySet )
{
   pUnkOuter;
   ppPropertySet;
   return ( E_NOTIMPL );
}

MIMEOLEAPI MimeOleGetCharsetInfo( /* in */ HCHARSET hCharset, /* out */ LPINETCSETINFO pCsetInfo )
{
   hCharset;
   pCsetInfo;
   return ( E_NOTIMPL );
}

MIMEOLEAPI MimeOleGetExtContentType( /* in */ LPCSTR pszExtension, /* out */ LPSTR* ppszContentType )
{
   pszExtension;
   ppszContentType;
   return ( E_NOTIMPL );
}

