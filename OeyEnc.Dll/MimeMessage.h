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
 
/*  MimeMessage.h
 *    IMimeMessage implementation for INETCOMM!MimeOleCreateMessage
 */

#pragma once

#define STRCONSTA(x,y)  EXTERN_C __declspec(selectany) const char x[] = y
#define STRCONSTW(x,y)  EXTERN_C __declspec(selectany) const WCHAR x[] = L##y
#define INITGUID
#include <guiddef.h>
#include <mimeole.h>
// {FD853CD8-7F86-11D0-8252-00C04FD85AB4}
DEFINE_GUID(IID_CMessageTree, 0xfd853cd8, 0x7f86, 0x11d0, 0x82, 0x52, 0x00, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

#ifdef UNICODE
// Saving predefined unicode macros
#pragma push_macro("GetProp")
#pragma push_macro("SetProp")
#undef GetProp
#undef SetProp
#endif // UNICODE

#define _STATICCRCTABLE
#include "yDecode.h"

#ifndef PAGE_SIZE
   #define PAGE_SIZE 4096
#endif /* PAGE_SIZE */

/**
 * X-OeyEnc
 *    Custom RFC822 header which holds message information after parsing. This 
 *    is used primarily for supporting combine+decode
 */
#define OEYENC_HEADER         "X-OeyEnc"
/* Default yEnc file extension */
#define OEYENC_YENCFILEEXT    ".ntx"
/* Filename to use when one can't be deciphered from the yEnc header */
#define OEYENC_UNKOWNFILE     "Unknown.ntx"
/* Filename of the decoder report when _APPENDREPORTS is enabled */
#define OEYENC_REPORTFILE     "OeyEnc Decoder Report.txt"
/* Registry value name of the optional message subject trigger */
#define OEYENC_REG_SUBJECT    "Subject"
/* Registry value name for a flag to control report generation */
#define OEYENC_REG_REPORT     "AttachReport"

/**
 *
 * Compilation flags for experimental/debugging code
 *
 **/

//#define _COPYATTACH
//#define _HANDSOFF
//#define _NOPARTSIZE
//#define _NOPARTCRC
//#define _NOPARTNUMBERS
//#define _NOCLEANTREEONSAVE
//#define _NOCOMBINE
//#define _NOCOMBINEMEMSTREAM
//#define _LOADMULTIPART
//#define _ENABLETRIMWORKSET
//#define _DUMPSTREAMPRELOAD
//#define _DUMPSTREAMPOSTLOAD
//#define _DUMPSTREAMREPORTS
#ifdef _DEBUG
   //#define _APPENDREPORTS
   //#define _TEXTBODYREPORTS
   #define _USEVERIFIERHEAP
#endif

class CMimeMessage;
class CMimeMessageFile;

/**
 * CMimeMessage
 *    A wrapper for IMimeMessageW.
 *
 * Remarks
 *    The only interface of real interest to this library should be
 *    IPersistStreamInit. However IMimeMessage took a design which made
 *    it part of the inheritance tree of the interface, rather than of the
 *    class object. This means the entire IMimeMessage interface must be
 *    implemented. Further, MIMEOLE doesn't really expose IMimeMessage from
 *    MimeOleCreateMessage but rather IMimeMessageW so that is what we really 
 *    implement. 
 *
 *    Because this implementation uses blind-aggregation of the contained
 *    message all locking is deferred to it with the exception of AddRef
 *    and Release which end up back here regardless.
 */
class ATL_NO_VTABLE CMimeMessage : public CComObjectRootEx<CComMultiThreadModel>,
                                   public IMimeMessageW,
                                   public IPersistFile
{
    /* ATL */
public:
   enum { eTraceCOMLevel = 4 };

   DECLARE_AGGREGATABLE(CMimeMessage)

   BEGIN_COM_MAP(CMimeMessage)
      COM_INTERFACE_ENTRY(IPersistStreamInit)
      COM_INTERFACE_ENTRY(IPersistFile)
      COM_INTERFACE_ENTRY(IMimeMessageTree)
      COM_INTERFACE_ENTRY(IMimeMessage)
      COM_INTERFACE_ENTRY(IMimeMessageW)
      /* Forward anything not in this interface map to the aggregated inner */
      COM_INTERFACE_ENTRY_AGGREGATE_BLIND(_pOeUnk)
   END_COM_MAP()

   DECLARE_GET_CONTROLLING_UNKNOWN()
   DECLARE_PROTECT_FINAL_CONSTRUCT()

   HRESULT FinalConstruct( ) throw();
	void FinalRelease( ) throw();

#ifdef _ATL_DEBUG_INTERFACES
   static HRESULT WINAPI InternalQueryInterface( void* pThis, const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject )
	{
      /* IID_CMessageTree is a special IID used by OE to reference the class instance. If _ATL_DEBUG_INTERFACES is defined 
       * ATL will create a thunk for it which OE will then try to reference as class data causing the app to blow up. To
       * resolve this, when IID_CMessageTree is requested the thunk is destroyed and the original object is returned */
      HRESULT hr = CComObjectRootBase::InternalQueryInterface(pThis, pEntries, iid, ppvObject);

      if ( SUCCEEDED(hr) && InlineIsEqualGUID(IID_CMessageTree, iid) )
      {
         _QIThunk* pThunk = (_QIThunk*)(*ppvObject);
         (*ppvObject) = pThunk->m_pUnk;
         _AtlDebugInterfacesModule.DeleteThunk(pThunk);
      }

      return ( hr );
	}
#endif /* _ATL_DEBUG_INTERFACES */

    /* IPersist */
public:
   STDMETHOD(GetClassID)( CLSID* pClassID ) throw();

   /*  IPersistStreamInit */
public:
   STDMETHOD(IsDirty)( ) throw();
   STDMETHOD(Load)( LPSTREAM pStm ) throw();
   STDMETHOD(Save)( LPSTREAM pStm,BOOL fClearDirty ) throw();
   STDMETHOD(GetSizeMax)( ULARGE_INTEGER* pCbSize ) throw();
   STDMETHOD(InitNew)( ) throw();

   /* IPersistFile */
public:
   STDMETHOD(Load)( LPCOLESTR pszFileName, DWORD dwMode ) throw();
   STDMETHOD(Save)( LPCOLESTR pszFileName, BOOL fRemember ) throw();
   STDMETHOD(SaveCompleted)( LPCOLESTR pszFileName ) throw();
   STDMETHOD(GetCurFile)( LPOLESTR* ppszFileName ) throw();   

   /* IMimeMessageTree */
public:
   STDMETHOD(GetMessageSource)( IStream** ppStream, DWORD dwFlags ) throw();
   STDMETHOD(GetMessageSize)( ULONG* pcbSize, DWORD dwFlags ) throw();
   STDMETHOD(LoadOffsetTable)( IStream* pStream ) throw();
   STDMETHOD(SaveOffsetTable)( IStream* pStream, DWORD dwFlags ) throw();
   STDMETHOD(GetFlags)( DWORD* pdwFlags ) throw();
   STDMETHOD(Commit)( DWORD dwFlags ) throw();
   STDMETHOD(HandsOffStorage)( ) throw();
   STDMETHOD(BindToObject)( const HBODY hBody, REFIID riid, void** ppvObject ) throw();
   STDMETHOD(SaveBody)( HBODY hBody, DWORD dwFlags, IStream* pStream ) throw();
   STDMETHOD(InsertBody)( BODYLOCATION location, HBODY hPivot, LPHBODY phBody ) throw();
   STDMETHOD(GetBody)( BODYLOCATION location, HBODY hPivot, LPHBODY phBody ) throw();
   STDMETHOD(DeleteBody)( HBODY hBody, DWORD dwFlags ) throw();
   STDMETHOD(MoveBody)( HBODY hBody, BODYLOCATION location ) throw();
   STDMETHOD(CountBodies)( HBODY hParent, boolean fRecurse, ULONG* pcBodies ) throw();
   STDMETHOD(FindFirst)( LPFINDBODY pFindBody, LPHBODY phBody ) throw();
   STDMETHOD(FindNext)( LPFINDBODY pFindBody, LPHBODY phBody ) throw();
   STDMETHOD(ResolveURL)( HBODY hRelated, LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, LPHBODY phBody ) throw();
   STDMETHOD(ToMultipart)( HBODY hBody, LPCSTR pszSubType, LPHBODY phMultipart ) throw();
   STDMETHOD(GetBodyOffsets)( HBODY hBody, LPBODYOFFSETS pOffsets ) throw();
   STDMETHOD(GetCharset)( LPHCHARSET phCharset ) throw();
   STDMETHOD(SetCharset)( HCHARSET hCharset, CSETAPPLYTYPE applytype ) throw();
   STDMETHOD(IsBodyType)( HBODY hBody, IMSGBODYTYPE bodytype ) throw();
   STDMETHOD(IsContentType)( HBODY hBody, LPCSTR pszPriType, LPCSTR pszSubType ) throw();
   STDMETHOD(QueryBodyProp)( HBODY hBody, LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive ) throw();
   STDMETHOD(GetBodyProp)( HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw();
   STDMETHOD(SetBodyProp)( HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw();
   STDMETHOD(DeleteBodyProp)( HBODY hBody, LPCSTR pszName ) throw();
   STDMETHOD(SetOption)( const TYPEDID oid, LPCPROPVARIANT pValue ) throw();
   STDMETHOD(GetOption)( const TYPEDID oid, LPPROPVARIANT pValue ) throw();

   /* IMimeMessage */
public:
   STDMETHOD(CreateWebPage)( IStream* pRootStm, LPWEBPAGEOPTIONS pOptions, IMimeMessageCallback* pCallback, IMoniker** ppMoniker ) throw();
   STDMETHOD(GetProp)( LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw();
   STDMETHOD(SetProp)( LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw();
   STDMETHOD(DeleteProp)( LPCSTR pszName ) throw();
   STDMETHOD(QueryProp)( LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive ) throw();
   STDMETHOD(GetTextBody)( DWORD dwTxtType, ENCODINGTYPE ietEncoding, IStream** ppStream, LPHBODY phBody ) throw();
   STDMETHOD(SetTextBody)( DWORD dwTxtType, ENCODINGTYPE ietEncoding, HBODY hAlternative, IStream* pStream, LPHBODY phBody ) throw();
   STDMETHOD(AttachObject)( REFIID riid, void* pvObject, LPHBODY phBody ) throw();
   STDMETHOD(AttachFile)( LPCSTR pszFilePath, IStream* pstmFile, LPHBODY phBody ) throw();
   STDMETHOD(AttachURL)( LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, IStream* pstmURL, LPSTR* ppszCIDURL, LPHBODY phBody ) throw();
   STDMETHOD(GetAttachments)( ULONG* pcAttach, LPHBODY* pprghAttach ) throw();
   STDMETHOD(GetAddressTable)( IMimeAddressTable** ppTable ) throw();
   STDMETHOD(GetSender)( LPADDRESSPROPS pAddress ) throw();
   STDMETHOD(GetAddressTypes)( DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList ) throw();
   STDMETHOD(GetAddressFormat)( DWORD dwAdrType, ADDRESSFORMAT format, LPSTR* ppszFormat ) throw();
   STDMETHOD(EnumAddressTypes)( DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes** ppEnum ) throw();
   STDMETHOD(SplitMessage)( ULONG cbMaxPart, IMimeMessageParts** ppParts ) throw();
   STDMETHOD(GetRootMoniker)( IMoniker** ppMoniker ) throw();

   /* IMimeMessageW */
public:
   STDMETHOD(GetPropW)( LPCWSTR pwszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw();
   STDMETHOD(SetPropW)( LPCWSTR pwszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw();
   STDMETHOD(DeletePropW)( LPCWSTR pwszName ) throw();
   STDMETHOD(QueryPropW)( LPCWSTR pwszName, LPCWSTR pwszCriteria, boolean fSubString, boolean fCaseSensitive ) throw();
   STDMETHOD(AttachFileW)( LPCWSTR pwszFilePath, IStream* pstmFile, LPHBODY phBody ) throw();
   STDMETHOD(AttachURLW)( LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, IStream* pstmURL, LPWSTR* ppwszCIDURL, LPHBODY phBody ) throw();
   STDMETHOD(GetAddressFormatW)( DWORD dwAdrType, ADDRESSFORMAT format, LPWSTR* ppwszFormat ) throw();
   STDMETHOD(ResolveURLW)( HBODY hRelated, LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, LPHBODY phBody ) throw();
   
   /* CMimeMessage */
public:
   static HRESULT _BindProvider( BOOL fAttach ) throw();

   /* Limit creation to CComXxxObject<> */
protected:
   CMimeMessage( ) throw();

   IUnknown*                     _pOeUnk;
   IMimeMessageW*                _pOeMsgW;

#ifdef _ENABLETRIMWORKSET
   SIZE_T                        _cbSetMin;
   SIZE_T                        _cbSetMax;
#endif /* _ENABLETRIMWORKSET */
   static LPCTSTR                _rgLibList[];
   static HMODULE                _rgLibCache[];
   static ULONG                  _iLibCacheRef;

   void InitializeModuleCache( ) throw();
   void UninitializeModuleCache( ) throw();

   static HRESULT __stdcall _MimeOleCreateMessage( IUnknown* pUnkOuter, IMimeMessage** ppMessage ) throw();
   
   static HRESULT _SetMessageOption( IMimeMessage* pMimeMessage, const TYPEDID oid, VARTYPE vt, void* pv ) throw();
   static HRESULT _SetBodyProp( IMimeMessage* pMimeMessage, HBODY hBody, LPCSTR pszName, DWORD dwFlags, VARTYPE vt, void* pv ) throw();
   static HRESULT _AppendBodyHeader( IMimeMessage* pMimeMessage, HBODY hBody, LPCSTR pszName, LPCWSTR pszValue ) throw();
   
   static HRESULT _AttachStream( IMimeMessage* pMimeMessage, IStream* pStream, ENCODINGTYPE ietEncoding, LPCSTR pszContentType, LPCWSTR pszFileName, USHORT iPart, USHORT iTotal, ULONG cbSize, LPHBODY phBody ) throw();
   static HRESULT _AppendTextStream( IMimeMessage* pMimeMessage, IStream* pStream, ENCODINGTYPE ietEncoding, DWORD dwTxtType, LPHBODY phBody ) throw();
   static HRESULT _LoadMessageStream( IMimeMessage* pMimeMessage, IStream* pStream, BOOL bHeadersOnly ) throw();
   
   static HRESULT _CombineRelatedAttachments( IMimeMessage* pMsgSource, IMimeMessage* pMsgTarget ) throw();
   static HRESULT _LoadEncodedStream( IMimeMessage* pMimeMessage, IStream* pStream ) throw();
   static HRESULT _CreateDecoderReport( IMimeMessage* pMimeMessage, LPCBYTE pbData, ULONG cbData, LPCWSTR pszFileName, USHORT iPart, USHORT iTotal, CRC32 iCrc, LPHBODY phBody ) throw();
   static HRESULT _CreateMimeStream( IStream** ppStream, bool bByteStream );

private:
   /* Copy-construction and copy-assignment are not supported */
   CMimeMessage( const CMimeMessage& );
   CMimeMessage& operator =( const CMimeMessage& );
};

typedef CComPolyObject<CMimeMessage> CoMimeMessage;

#ifdef UNICODE
// Restoring predefined unicode macros
#pragma pop_macro("GetProp")
#pragma pop_macro("SetProp")
#endif // UNICODE

