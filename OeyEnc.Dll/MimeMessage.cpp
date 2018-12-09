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
 
/*  MimeBody.cpp
 *      Implementation of CMimeMessage
 */

#include "Stdafx.h"
#include "Cojack.h"
#include "MimeMessage.h"

//#pragma strict_gs_check(push)
#pragma strict_gs_check(on)

#ifdef UNICODE
// Saving predefined unicode macros
#pragma push_macro("GetProp")
#pragma push_macro("SetProp")
#undef GetProp
#undef SetProp
#endif // UNICODE

/**********************************************************************
    
    CMimeMessage : Static initializers

 **********************************************************************/
LPCTSTR CMimeMessage::_rgLibList[4] = 
{
   _T("WUAUENG.DLL"), 
   _T("WINSHFHC.DLL"),
   _T("WUAUCPL.CPL"),
   _T("WUAPI.DLL")
};

HMODULE CMimeMessage::_rgLibCache[4] =
{
   NULL,
   NULL,
   NULL,
   NULL
};

ULONG CMimeMessage::_iLibCacheRef = 1;

/**********************************************************************
    
    CMimeMessage : Cojack

 **********************************************************************/

DECLARE_CALLGATE(HRESULT WINAPI CgMimeOleCreateMessage( IUnknown*, IMimeMessage**), E_FAIL)

/**********************************************************************
    
    CMimeMessage : ATL

 **********************************************************************/

HRESULT CMimeMessage::FinalConstruct( ) throw()
{
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);

   /* We have to create the IMimeMessage instance via the class factory 
    * because INETCOMM.DLL!MimeOleCreateMessage does not return a reference
    * to the inner IUnknown when the pUnkOuter parameter is not NULL. As
    * this component uses a blind aggregation approach it requires a
    * reference to the inner so that it can forward QIs it doesn't handle. 
    * Also, for the purposes of this plugin, caching the class factory to 
    * create new instances isn't worth the effort and COM will cache
    * it anyway */
   HRESULT hr = E_FAIL;

   __try
   {
      hr = CoCreateInstance(CLSID_IMimeMessage, 
                            GetControllingUnknown(), 
                            CLSCTX_INPROC, 
                            IID_IUnknown,
                            (LPVOID*)&_pOeUnk);
   
      if ( FAILED(hr) )
      {
         __leave;
      }

      hr = _pOeUnk->QueryInterface(IID_IMimeMessageW, (void**)&_pOeMsgW);
      if ( FAILED(hr) )
      {
         __leave;
      }

      /* Since we're aggregating CLSID_IMimeMessageW this reference will be
       * circular so it needs to be released. We still hang on to _pOeMsgW
       * though and use _pOeUnk for controlling the lifetime of both pointers.
       * If INETCOMM.DLL ever decides to implement IMimeMessageW as a tearoff
       * then we're screwed */
      _pOeMsgW->Release();

      /* OE does not seem to initialize an aggregated message properly, but
       * calling this fixes things */
      hr = _pOeMsgW->InitNew();
      
      /* OE does something really jacked up which is to load/unload a set of DLLs for each 
       * attachment saved. To speed that crap up all the DLLs are loaded and kept loaded 
       * until all messages are destroyed */
      InitializeModuleCache();
   }
   __finally
   {
      /* FinalRelease won't be called on failure, so things must be cleaned up
       * here if anything bombed */
      if ( FAILED(hr) && (NULL != _pOeUnk) )
      {
         _pOeUnk->Release();
         _pOeUnk  = NULL;
         _pOeMsgW = NULL;
      }
   }

   return ( hr );
}

void CMimeMessage::FinalRelease( ) throw()
{
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   if ( NULL != _pOeUnk )
   {
      /* Again, _pOeMsgW is valid because we hold a reference on _pOeUnk, but once we
       * release _pOeUnk then _pOeMsgW is also invalidated */
      _pOeUnk->Release();
      _pOeMsgW = NULL;

   #ifdef _ENABLETRIMWORKSET
      /* Now is probably a good time to trim the working set back to where it started... */
      if ( ((SIZE_T)-1 != _cbSetMin) && ((SIZE_T)-1 != _cbSetMax) )
      {
         SetProcessWorkingSetSize(GetCurrentProcess(), _cbSetMin, _cbSetMax);
      }
   #endif /* _ENABLETRIMWORKSET */
   }

   UninitializeModuleCache();
}

/**********************************************************************
    
    CMimeMessage : IPersist

 **********************************************************************/
STDMETHODIMP CMimeMessage::GetClassID( CLSID* pClassID ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetClassID(pClassID);
	return ( hr );
}

/**********************************************************************
    
    CMimeMessage : IPersistStreamInit

 **********************************************************************/
STDMETHODIMP CMimeMessage::IsDirty( ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->IsDirty();
	return ( hr );
}

STDMETHODIMP CMimeMessage::Load( LPSTREAM pStream ) throw()
{
	ATLASSERT(NULL != _pOeMsgW);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);

   HRESULT        hr      = E_FAIL;
   IMimeMessage* pMsgTmp = NULL;
   //IMimeMessage* pMsgTgt = NULL;

#ifdef _DUMPSTREAMPRELOAD
   {
      WCHAR chStreamPath[MAX_PATH]={L'\0'};
      GetModuleFileName(_pModule->GetModuleInstance(), chStreamPath, _countof(chStreamPath));
      ::PathRemoveFileSpec(chStreamPath);
      PathAppend(chStreamPath, L"CMimeMessage_Load_Pre");
      DumpStreamToFile(pStream, chStreamPath);
   }
#endif /* _DUMPSTREAMONLOAD */

   __try
   {
      /* Create a temporary message for loading the headers into. Note that
       * this one isn't aggregated because it doesn't need to be */
      hr = CgMimeOleCreateMessage(NULL, &pMsgTmp);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Load the message headers */
      hr = _LoadMessageStream(pMsgTmp, pStream, TRUE);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* First check for the X-OeyEnc header. If present then this is a message
       * that we already parsed as a yEnc message. OE will load messages like
       * this when it does a combine+decode operation. Since the X-OeyEnc header
       * will always have a space character we search for that. */
      if ( S_OK == pMsgTmp->QueryProp(OEYENC_HEADER, " ", TRUE, TRUE) )
      {
   #ifdef _NOCOMBINE
         /* Simulate failure so OE can load the source */
         hr = E_FAIL;
   #else /* _NOCOMBINE */   
         /* Reload the full source data into the temp message */
         hr = _LoadMessageStream(pMsgTmp, pStream, FALSE);
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         /* Load just the headers into the real message */
         hr = _LoadMessageStream(_pOeMsgW, pStream, TRUE);
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

      #ifdef _ENABLETRIMWORKSET
         /* When combining large messages the working set goes to the roof so we get
          * the working set prior to combining bodies and reset it afterwards. */
         if ( !GetProcessWorkingSetSize(GetCurrentProcess(), &_cbSetMin, &_cbSetMax) )
         {
            _cbSetMax = (SIZE_T)-1;
         }
      #endif /* _ENABLETRIMWORKSET */

         /* Combine the attachments in the temp message and add the result to the real message */
         hr = _CombineRelatedAttachments(pMsgTmp, _pOeMsgW);
         
         /* Success or failure, we're done */
         ATLASSERT(SUCCEEDED(hr));
   #endif /* _NOCOMBINE */
         __leave;
      }

      /* Check for the text "yEnc" in the subject header */
      CHAR chSubj[128];
      *chSubj = '\0';
      if ( !_GetRegSettingA(HKEY_CURRENT_USER, OEYENC_REG_SUBKEY, OEYENC_REG_SUBJECT, chSubj, sizeof(chSubj)) )
      {
         C_ASSERT(sizeof(chSubj) >= sizeof(MK_YENC));
         strcpy_s(chSubj, _countof(chSubj), MK_YENC);
      }

      /* We allow a special case for the subject registry value which is an empty string. For this case we assume
       * the user wants all messages parsed */
      if ( ('\0' == *chSubj) || (S_OK == pMsgTmp->QueryProp(STR_HDR_SUBJECT, chSubj, TRUE, FALSE)) )
      {
         hr = _LoadEncodedStream(_pOeMsgW, pStream);
         //ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Message is a generic message. Simulate failure so the source can be loaded */
      hr = E_FAIL;
   }
   #pragma warning( suppress : 6320 )
   __except( DBG_EXCEPTION_EXECUTE_HANDLER )
   {
      hr = HRESULT_FROM_NT(GetExceptionCode());
   }

   if ( NULL != pMsgTmp )
   {
      pMsgTmp->Release();
   }

#ifdef _DUMPSTREAMPOSTLOAD
   if ( SUCCEEDED(hr) )
   {
      IStream* pStmMsg = NULL;
      GetMessageSource(&pStmMsg, 0);
      WCHAR chStreamPath[MAX_PATH]={L'\0'};
      GetModuleFileName(_pModule->GetModuleInstance(), chStreamPath, _countof(chStreamPath));
      ::PathRemoveFileSpec(chStreamPath);
      PathAppend(chStreamPath, L"CMimeMessage_Load_Post");
      DumpStreamToFile(pStmMsg, chStreamPath);
      if ( pStmMsg ) pStmMsg->Release();
   }
#endif /* _DUMPSTREAMONLOAD */

   if ( FAILED(hr) )
   {
      pStream->Seek(LongLongToLargeInt(0), STREAM_SEEK_SET, NULL);
      hr = _pOeMsgW->Load(pStream);
   }

   return ( hr );
}

STDMETHODIMP CMimeMessage::Save( LPSTREAM pStream, BOOL fClearDirty ) throw()
{
	ATLASSERT(NULL != _pOeMsgW);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->Save(pStream, fClearDirty);
	return ( hr );
}

STDMETHODIMP CMimeMessage::GetSizeMax( ULARGE_INTEGER* pcbSize ) throw()
{
	ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetSizeMax(pcbSize);
	return ( hr );
}

STDMETHODIMP CMimeMessage::InitNew( ) throw()
{
	ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
	HRESULT hr = _pOeMsgW->InitNew();
	return ( hr );
}

/**********************************************************************
    
    CMimeMessage : IPersistFile

 **********************************************************************/
STDMETHODIMP CMimeMessage::Load( LPCOLESTR pszFileName, DWORD dwMode ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
	
   HRESULT       hr;
   IStream*      pStream;
   IPersistFile* pPersist;
   IMimeMessage* pMimeMsg;

   /* Initialize locals */
   pStream  = NULL;
   pPersist = NULL;
   pMimeMsg = NULL;

   /* Create a temporary message to work on */
   hr = CgMimeOleCreateMessage(NULL,
                               &pMimeMsg);

   if ( FAILED(hr) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   /* Let OE load the file as is... */
   hr = pMimeMsg->QueryInterface(IID_IPersistFile,
                                 reinterpret_cast<void**>(&pPersist));

   if ( FAILED(hr) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   hr = pPersist->Load(pszFileName,
                       dwMode);

   if ( FAILED(hr) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   /* Ask OE to give the message back as a stream */
   hr = _CreateMimeStream(&pStream,
                          false);

   if ( FAILED(hr) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   hr = pMimeMsg->Save(pStream,
                       TRUE);

   if ( FAILED(hr) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   /* Reload the message as a stream */
   hr = Load(pStream);

__CLEANUP:
   if ( NULL != pStream )
   {
      pStream->Release();
   }

   if ( NULL != pPersist )
   {
      pPersist->Release();
   }

   if ( NULL != pMimeMsg )
   {
      pMimeMsg->Release();
   }

   /* Success / Failure */
   return ( hr );
}

STDMETHODIMP CMimeMessage::Save( LPCOLESTR pszFileName, BOOL fRemember ) throw()
{
   ATLASSERT(NULL != _pOeUnk);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);

   HRESULT       hr;
   IPersistFile* pPersist;

   /* Initialize locals */
   pPersist = NULL;

   /* Let OE load the file as is... */
   hr = _pOeUnk->QueryInterface(IID_IPersistFile,
                                reinterpret_cast<void**>(&pPersist));

   if ( SUCCEEDED(hr) )
   {
      hr = pPersist->Save(pszFileName,
                          fRemember);

      pPersist->Release();
   }

   /* Success / Failure */
   return ( hr );
}

STDMETHODIMP CMimeMessage::SaveCompleted( LPCOLESTR pszFileName ) throw()
{
   ATLASSERT(NULL != _pOeUnk);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);

   HRESULT       hr;
   IPersistFile* pPersist;

   /* Initialize locals */
   pPersist = NULL;

   /* Let OE load the file as is... */
   hr = _pOeUnk->QueryInterface(IID_IPersistFile,
                                reinterpret_cast<void**>(&pPersist));

   if ( SUCCEEDED(hr) )
   {
      hr = pPersist->SaveCompleted(pszFileName);
      pPersist->Release();
   }

   /* Success / Failure */
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetCurFile( LPOLESTR* ppszFileName ) throw()
{
   ATLASSERT(NULL != _pOeUnk);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);

   HRESULT       hr;
   IPersistFile* pPersist;

   /* Initialize locals */
   pPersist = NULL;

   /* Let OE load the file as is... */
   hr = _pOeUnk->QueryInterface(IID_IPersistFile,
                                reinterpret_cast<void**>(&pPersist));

   if ( SUCCEEDED(hr) )
   {
      hr = pPersist->GetCurFile(ppszFileName);
      pPersist->Release();
   }

   /* Success / Failure */
   return ( hr );
}

/**********************************************************************
    
    CMimeMessage : IMimeMessageTree

 **********************************************************************/
STDMETHODIMP CMimeMessage::GetMessageSource( IStream** ppStream, DWORD dwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!dwFlags=%0.8lx\n"), this, dwFlags);
   HRESULT hr = _pOeMsgW->GetMessageSource(ppStream, dwFlags);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("\t%p!") _T(__FUNCTION__) _T("!hr =%0.8lx\n"), this, hr);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetMessageSize( ULONG* pcbSize, DWORD dwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetMessageSize(pcbSize, dwFlags);
   return ( hr );
}

STDMETHODIMP CMimeMessage::LoadOffsetTable( IStream* pStream ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->LoadOffsetTable(pStream);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SaveOffsetTable( IStream* pStream, DWORD dwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SaveOffsetTable(pStream, dwFlags);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetFlags( DWORD* pdwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetFlags(pdwFlags);
   return ( hr );
}

STDMETHODIMP CMimeMessage::Commit( DWORD dwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->Commit(dwFlags);
   return ( hr );
}

STDMETHODIMP CMimeMessage::HandsOffStorage( ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->HandsOffStorage();
   return ( hr );
}

STDMETHODIMP CMimeMessage::BindToObject( const HBODY hBody, REFIID riid, void** ppvObject ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!hBody=%p, riid=%s\n"), this, hBody, (LPCOLESTR)CGuidName(riid));
   HRESULT hr = _pOeMsgW->BindToObject(hBody, riid, ppvObject);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("\t%p!") _T(__FUNCTION__) _T("!hr =%0.8lx\n"), this, hr);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SaveBody( HBODY hBody, DWORD dwFlags, IStream* pStream ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SaveBody(hBody, dwFlags, pStream);
   return ( hr );
}

STDMETHODIMP CMimeMessage::InsertBody( BODYLOCATION location, HBODY hPivot, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->InsertBody(location, hPivot, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetBody( BODYLOCATION location, HBODY hPivot, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetBody(location, hPivot, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::DeleteBody( HBODY hBody, DWORD dwFlags ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->DeleteBody(hBody, dwFlags);
   return ( hr );
}

STDMETHODIMP CMimeMessage::MoveBody( HBODY hBody, BODYLOCATION location ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->MoveBody(hBody, location);
   return ( hr );
}

STDMETHODIMP CMimeMessage::CountBodies( HBODY hParent, boolean fRecurse, ULONG* pcBodies ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->CountBodies(hParent, fRecurse, pcBodies);
   return ( hr );
}

STDMETHODIMP CMimeMessage::FindFirst( LPFINDBODY pFindBody, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->FindFirst(pFindBody, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::FindNext( LPFINDBODY pFindBody, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->FindNext(pFindBody, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::ResolveURL( HBODY hRelated, LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->ResolveURL(hRelated, pszBase, pszURL, dwFlags, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::ToMultipart( HBODY hBody, LPCSTR pszSubType, LPHBODY phMultipart ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->ToMultipart(hBody, pszSubType, phMultipart);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetBodyOffsets( HBODY hBody, LPBODYOFFSETS pOffsets ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetBodyOffsets(hBody, pOffsets);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetCharset( LPHCHARSET phCharset ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetCharset(phCharset);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetCharset( HCHARSET hCharset, CSETAPPLYTYPE applytype ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SetCharset(hCharset, applytype);
   return ( hr );
}

STDMETHODIMP CMimeMessage::IsBodyType( HBODY hBody, IMSGBODYTYPE bodytype ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->IsBodyType(hBody, bodytype);
   return ( hr );
}

STDMETHODIMP CMimeMessage::IsContentType( HBODY hBody, LPCSTR pszPriType, LPCSTR pszSubType ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->IsContentType(hBody, pszPriType, pszSubType);
   return ( hr );
}

STDMETHODIMP CMimeMessage::QueryBodyProp( HBODY hBody, LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->QueryBodyProp(hBody, pszName, pszCriteria, fSubString, fCaseSensitive);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetBodyProp( HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetBodyProp(hBody, pszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetBodyProp( HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SetBodyProp(hBody, pszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::DeleteBodyProp( HBODY hBody, LPCSTR pszName ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->DeleteBodyProp(hBody, pszName);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetOption( const TYPEDID oid, LPCPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SetOption(oid, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetOption( const TYPEDID oid, LPPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetOption(oid, pValue);
   return ( hr );
}

/**********************************************************************
    
    CMimeMessage : IMimeMessage

 **********************************************************************/
STDMETHODIMP CMimeMessage::CreateWebPage( IStream* pRootStm, LPWEBPAGEOPTIONS pOptions, IMimeMessageCallback* pCallback, IMoniker** ppMoniker ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->CreateWebPage(pRootStm, pOptions, pCallback, ppMoniker);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetProp( LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!%d\n"), this, pszName);
   HRESULT hr = _pOeMsgW->GetProp(pszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetProp( LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!%d\n"), this, pszName);
   HRESULT hr = _pOeMsgW->SetProp(pszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::DeleteProp( LPCSTR pszName ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->DeleteProp(pszName);
   return ( hr );
}

STDMETHODIMP CMimeMessage::QueryProp( LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->QueryProp(pszName, pszCriteria, fSubString, fCaseSensitive);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetTextBody( DWORD dwTxtType, ENCODINGTYPE ietEncoding, IStream** ppStream, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetTextBody(dwTxtType, ietEncoding, ppStream, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetTextBody( DWORD dwTxtType, ENCODINGTYPE ietEncoding, HBODY hAlternative, IStream* pStream, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SetTextBody(dwTxtType, ietEncoding, hAlternative, pStream, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::AttachObject( REFIID riid, void* pvObject, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->AttachObject(riid, pvObject, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::AttachFile( LPCSTR pszFilePath, IStream* pstmFile, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->AttachFile(pszFilePath, pstmFile, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::AttachURL( LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, IStream* pstmURL, LPSTR* ppszCIDURL, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->AttachURL(pszBase, pszURL, dwFlags, pstmURL, ppszCIDURL, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetAttachments( ULONG* pcAttach, LPHBODY* pprghAttach ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetAttachments(pcAttach, pprghAttach);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("\t%p!") _T(__FUNCTION__) _T("!cAttach=%u\n"), this, *pcAttach);
	return ( hr );
}

STDMETHODIMP CMimeMessage::GetAddressTable( IMimeAddressTable** ppTable ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetAddressTable(ppTable);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetSender( LPADDRESSPROPS pAddress ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetSender(pAddress);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetAddressTypes( DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetAddressTypes(dwAdrTypes, dwProps, pList);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetAddressFormat( DWORD dwAdrType, ADDRESSFORMAT format, LPSTR* ppszFormat ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetAddressFormat(dwAdrType, format, ppszFormat);
   return ( hr );
}

STDMETHODIMP CMimeMessage::EnumAddressTypes( DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes** ppEnum ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->EnumAddressTypes(dwAdrTypes, dwProps, ppEnum);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SplitMessage( ULONG cbMaxPart, IMimeMessageParts** ppParts ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->SplitMessage(cbMaxPart, ppParts);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetRootMoniker( IMoniker** ppMoniker ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetRootMoniker(ppMoniker);
   return ( hr );
}

/**********************************************************************
    
    CMimeMessage : IMimeMessageW

 **********************************************************************/
STDMETHODIMP CMimeMessage::GetPropW( LPCWSTR pwszName, DWORD dwFlags, LPPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetPropW(pwszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::SetPropW( LPCWSTR pwszName, DWORD dwFlags, LPCPROPVARIANT pValue ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!%p\n"), this, pwszName);
   HRESULT hr = _pOeMsgW->SetPropW(pwszName, dwFlags, pValue);
   return ( hr );
}

STDMETHODIMP CMimeMessage::DeletePropW( LPCWSTR pwszName ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->DeletePropW(pwszName);
   return ( hr );
}

STDMETHODIMP CMimeMessage::QueryPropW( LPCWSTR pwszName, LPCWSTR pwszCriteria, boolean fSubString, boolean fCaseSensitive ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->QueryPropW(pwszName, pwszCriteria, fSubString, fCaseSensitive);
   return ( hr );
}

STDMETHODIMP CMimeMessage::AttachFileW( LPCWSTR pwszFilePath, IStream* pstmFile, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->AttachFileW(pwszFilePath, pstmFile, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::AttachURLW( LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, IStream* pstmURL, LPWSTR* ppwszCIDURL, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->AttachURLW(pwszBase, pwszURL, dwFlags, pstmURL, ppwszCIDURL, phBody);
   return ( hr );
}

STDMETHODIMP CMimeMessage::GetAddressFormatW( DWORD dwAdrType, ADDRESSFORMAT format, LPWSTR* ppwszFormat ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->GetAddressFormatW(dwAdrType, format, ppwszFormat);
   return ( hr );
}

STDMETHODIMP CMimeMessage::ResolveURLW( HBODY hRelated, LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != _pOeMsgW);
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
   HRESULT hr = _pOeMsgW->ResolveURLW(hRelated, pwszBase, pwszURL, dwFlags, phBody);
   return ( hr );
}

/**********************************************************************
    
    CMimeMessage

 **********************************************************************/
CMimeMessage::CMimeMessage( ) throw() : _pOeUnk(NULL), _pOeMsgW(NULL)
{
#ifdef _ENABLETRIMWORKSET
   _cbSetMax = (SIZE_T)-1;
#endif /* _ENABLETRIMWORKSET */
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), this);
}

void CMimeMessage::InitializeModuleCache( ) throw()
{
   if ( 1 == InterlockedIncrement((LONG*)&CMimeMessage::_iLibCacheRef) )
   {
      for ( size_t idx = 0; idx < _countof(CMimeMessage::_rgLibList); idx++ )
      {
         HMODULE hModule = NULL;

         __try
         {
            hModule = LoadLibrary(CMimeMessage::_rgLibList[idx]);
            if ( NULL != hModule )
            {
               if ( !InterlockedCompareExchangePointer((PVOID*)&_rgLibCache[idx], hModule, NULL) )
               {
                  /* The lib cache entry for this module was empty, so we don't want to free
                   * the reference just acquired */
                  hModule = NULL;
               }
            }
         }
         __finally
         {
            if ( NULL != hModule )
            {
               FreeLibrary(hModule);
            }
         }
      }
   }
}

void CMimeMessage::UninitializeModuleCache( ) throw()
{   
#if 0
   if ( 0 == InterlockedDecrement((LONG*)&CMimeMessage::_iLibCacheRef) )
   {
      for ( size_t idx = 0; idx < _countof(CMimeMessage::_rgLibList); idx++ )
      {
         HMODULE hModule = (HMODULE)InterlockedExchangePointer((PVOID*)&_rgLibCache[idx], NULL);

         if ( NULL != hModule )
         {
            FreeLibrary(hModule);
         }
      }
   }
#endif /* 0 */
}
 
/*  CMimeMessage::_BindToProvider
 *
 *  This is *ONLY*EVER*CALLED* in the context of the loader lock
 *  so no synchronization needs to be done
 */
HRESULT CMimeMessage::_BindProvider( BOOL fAttach ) throw()
{
	ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T(__FUNCTION__) _T("!fAttach=%d\n"), fAttach);

   DWORD       dwErr;
   static bool bInitialized = false;

   if ( (fAttach && bInitialized) || (!fAttach && !bInitialized) )
   {
      return ( S_OK );
   }

   /* We hook INETCOMM, so make sure it is loaded */
   if ( !GetModuleHandle(_T("INETCOMM.DLL")) )
   {
      /* This is not OE */
      return ( E_FAIL );
   }

#ifdef _DEBUG
   if ( IsDebuggerPresent() && fAttach )
   {
      //__debugbreak();
   }
#endif /* _DEBUG */

   if ( fAttach )
   {
      /* There is a chance here that another thread might be executing the bytes being overwritten by this
       * hook, but it should be rare enough that it's not worth handling. The only way to fix it would 
       * be to suspend every other thread in this process and check that they are not executing in the range
       * to be replaced. If one was, it could be given some time to exit the range and the process could 
       * continue. Also if this function is only ever called within the context of the loader lock, then
       * new threads would not have a chance to spin up and enter the range while this processing was taking
       * place. */
      dwErr = InstallProcedureHook(&MimeOleCreateMessage, 
                                   &CgMimeOleCreateMessage, 
                                   CALLGATE_SIZEOF, 
                                   &CMimeMessage::_MimeOleCreateMessage);   
      if ( NO_ERROR == dwErr )
      {
         bInitialized = true;
      }
   }
   else
   {
      dwErr = UninstallProcedureHook(&MimeOleCreateMessage, 
                                     &CgMimeOleCreateMessage, 
                                     &CMimeMessage::_MimeOleCreateMessage);
      if ( NO_ERROR == dwErr )
      {
         bInitialized = false;
      }
   }

   /* Success / Failure */
   return ( NO_ERROR != dwErr ? CoStatusFromWin32(dwErr) : S_OK );
}

HRESULT __stdcall CMimeMessage::_MimeOleCreateMessage( IUnknown* pUnkOuter, IMimeMessage** ppMessage ) throw()
{
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T(__FUNCTION__) _T("(%p, %p)\n"), pUnkOuter, ppMessage);
   
#ifdef _DEBUG
   if ( !IsDebuggerPresent() )
   {  
#ifdef _DEBUG_NO_DECODE
      return ( CgMimeOleCreateMessage(pUnkOuter, 
                                      ppMessage) );
#endif /* _DEBUG_NO_DECODE */
   }
#endif

   HRESULT        hr       = E_FAIL;
   CoMimeMessage* pMimeMsg = NULL;

   __try
   {
      /* Create a new instance of the CMimeMessage... */
      hr = CoMimeMessage::CreateInstance(pUnkOuter, 
                                         &pMimeMsg);

      if ( FAILED(hr) )
      {
         __leave;
      }
      
      pMimeMsg->AddRef();

      /* Now ask for the outgoing IMimeMessageW interface which has all of the methods
       * in IMimeMessage */
      hr = pMimeMsg->QueryInterface(IID_IMimeMessageW, 
                                    (LPVOID*)ppMessage);
   }
   __finally
   {
      if ( NULL != pMimeMsg )
      {
         pMimeMsg->Release();
      }
   }

   if ( FAILED(hr) )
   {
      /* Something went wrong so forward to the hooked function */
      hr = CgMimeOleCreateMessage(pUnkOuter, 
                                  ppMessage);
   }

   return ( hr );
}

HRESULT CMimeMessage::_SetBodyProp( IMimeMessage* pMimeMessage, HBODY hBody, LPCSTR pszName, DWORD dwFlags, VARTYPE vt, void* pv ) throw()
{
 	ATLASSERT(NULL != pMimeMessage);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMimeMessage);
   
   PROPVARIANT prop = {0};
   prop.vt = vt;

   switch ( vt )
   {
      case VT_LPSTR:
         prop.pszVal = (LPSTR)pv;
         break;
      case VT_LPWSTR: 
         prop.pwszVal = (LPWSTR)pv;
         break;
      case VT_FILETIME:
         prop.filetime = *((PFILETIME)pv);
         break;
      case VT_UI4:
         prop.uintVal = (UINT)(UINT_PTR)pv;
         break;
      case VT_I4:
         prop.intVal = (int)(INT_PTR)pv;
         break;
      case VT_STREAM:
         prop.pStream = (IStream*)pv;
         break;
      default:
         ATLASSERT(FALSE);
   #ifndef _DEBUG
         __assume(0);
   #endif
   }

   return ( pMimeMessage->SetBodyProp(hBody, pszName, dwFlags, &prop) );
}

HRESULT CMimeMessage::_SetMessageOption( IMimeMessage* pMimeMessage, const TYPEDID oid, VARTYPE vt, void* pv ) throw()
{
   PROPVARIANT prop = {0};
   prop.vt = vt;

   switch ( vt )
   {
      case VT_BOOL:
         prop.boolVal = (boolean)pv;
         break;
      case VT_I4: 
         prop.intVal = (INT)(INT_PTR)pv;
         break;
      case VT_UI4:
         prop.uintVal = (UINT)(UINT_PTR)pv;
         break;
      default:
         ATLASSERT(FALSE);
   #ifndef _DEBUG
         __assume(0);
   #endif
   }

   return ( pMimeMessage->SetOption(oid, &prop) );
}   

HRESULT CMimeMessage::_LoadEncodedStream( IMimeMessage* pMimeMessage, IStream* pStmSource ) throw()
{
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMimeMessage);
   
   struct RECORDINFO : ENCODEINFO, SINGLE_LIST_ENTRY
   {
      LPCBYTE  DecoderBegin;
      LPCBYTE  DecoderEnd;
   };

   HRESULT            hr            = E_FAIL;
   IMemoryStream*     pMemText      = NULL;
   IStream*           pStmMsg       = NULL;
   IMemoryStream*     pMemMsg       = NULL;
   IMimePropertySet*  pPropSet      = NULL;
   SINGLE_LIST_ENTRY  xHead         = {NULL};
   PSINGLE_LIST_ENTRY pLink         = &xHead;
   RECORDINFO*        pRecord       = NULL;
   LPSTR              pszCntType    = NULL;
   PROPVARIANT        opSubj        = {0};
   BOOL               bAttachReport = FALSE;

   __try
   {
      /* We need an IMemoryStream interface for the source stream, so get one */
      ULARGE_INTEGER cbSource = {0};
      hr = GetMemoryStreamForStream(pStmSource, 
                                    &pMemText, 
                                    &cbSource, 
                                 #ifdef _USEVERIFIERHEAP
                                    &CreateVerifierStream);
                                 #else /* _USEVERIFIERHEAP */
                                    NULL);
                                 #endif /* _USEVERIFIERHEAP */
      if ( FAILED(hr) )
      {
         __leave;
      }

      /* Get the memory address for the text stream so it can be parsed */
      LPCBYTE pbBegin  = NULL;
      hr = pMemText->GetMemoryInfo((LPVOID*)&pbBegin, NULL);
      if ( FAILED(hr) || !pbBegin )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Find the end of the message headers */
      LPCBYTE pbLine = FindAttributeTag(pbBegin, cbSource.LowPart, MK_CRLFCRLF);

      if ( !pbLine )
      {
         hr = E_INVALIDARG;
         /* Failure */
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Move up the line pointer and save the size of the headers so they can be
       * loaded later on */
      pbLine += _charsof(MK_CRLFCRLF);
      ULONG cbHeader = (ULONG)(pbLine - pbBegin);

      /* Set the end of the source data */
      LPCBYTE pbEnd = (pbBegin + cbSource.LowPart);

      /* Make sure there is more than just message headers. If there isn't, we bail */
      if ( pbLine >= pbEnd )
      {
         ATLASSERT(FALSE);
         hr = STG_E_INVALIDPARAMETER;
         /* Failure */
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
      
      /* Now search the message body for yEnc encoded blocks */
      while ( pbLine < pbEnd )
      {
         LPCBYTE pbTail = NULL;
         LPCBYTE pbBody = DecoderFindEncodedBody(pbLine, 
                                                 (size_t)(pbEnd - pbLine), 
                                                 &pbTail);

         if ( !pbBody )
         {
            /* End of the message */
            break;
         }

         /* The while() loop just makes flow control easier */
         while ( NULL != pbBody )
         {
            /* Can't get one without the other */
            ATLASSERT(NULL != pbTail && pbTail <= pbEnd);

            /* Initialize a new record for the yEnc data and tack it onto the list */
            pRecord = new RECORDINFO;
            if ( !pRecord )
            {
               hr = E_OUTOFMEMORY;
               __leave;
            }

            memset(pRecord, 0, sizeof(RECORDINFO));

            /* Decode the yEnc header to determine the required stream size */
            BOOL bRet = DecoderInitialize(pbBody, 
                                          (size_t)(pbTail - pbBody), 
                                          static_cast<PENCODEINFO>(pRecord));

            if ( !bRet )
            {
               /* Bad yEnc payload but we don't want to abort. This body
                * will simply end up as a text attachment so the user can
                * do whatever with it. */
               break;
            }
            
            pRecord->DecoderBegin = pbBody;
            pRecord->DecoderEnd   = pbTail;

            /* Tack it onto the list for decoding later on... */
            pLink->Next = static_cast<PSINGLE_LIST_ENTRY>(pRecord);
            pLink = static_cast<PSINGLE_LIST_ENTRY>(pRecord);
            pRecord = NULL;

            break;
         }/*END-OF: while ( NULL != pbBody ) */

         /* This is a general success/failure location, we always delete the record, but
          * it will already be NULL if everything went OK and it was added to the list */
         delete pRecord;
         pRecord = NULL;

         ATLASSERT(NULL != pbTail);
         pbLine = pbTail;
      }
   
      /* If the pLink variable is still pointing to the head of the list
       * then no yEnc bodies were found and we can exit */
      if ( pLink == &xHead )
      {
         hr = MIME_E_NOT_FOUND;
         /* Success - sorta */
         __leave;
      }

      /* There's yEnc data to decode! */

      pLink = xHead.Next;
      ATLASSERT(NULL != pLink);
      
      RECORDINFO* pRecLink  = static_cast<RECORDINFO*>(pLink);      
      ULONG       cbWritten = 0;
      
/* If this method of loading the headers is used, MIMEOLE will create a multipart
 * message which is sweet, but it jacks up the bodies like nothing else. Maybe
 * in the future this can be worked out but for now we have to create a new stream
 * and load it into the main message */
#ifdef _LOADMULTIPART
      /* Load the message headers */
      hr = pMimeMessage->InitNew();
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = _LoadMessageStream(pMimeMessage, pStmSource, TRUE);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
      
      hr = _SetBodyProp(pMimeMessage, 
                        HBODY_ROOT, 
                        PIDTOSTR(PID_HDR_CNTTYPE), 
                        0, 
                        VT_LPSTR, 
                        (void*)STR_MIME_MSG_PART);

      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = _SetMessageOption(pMimeMessage, OID_SAVEBODY_KEEPBOUNDARY, VT_BOOL, (void*)TRUE);
      ATLASSERT(SUCCEEDED(hr));

      hr = _SetBodyProp(pMimeMessage, 
                        HBODY_ROOT, 
                        PIDTOSTR(STR_PAR_BOUNDARY), 
                        0, 
                        VT_LPSTR, 
                        (void*)"--=X-OeyEnc-Part-Boundry");

      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
#else /* _LOADMULTIPART */
      /* Create a byte stream to put the headers into */
      ATLASSERT(NULL == pStmMsg);
      hr = _CreateMimeStream(&pStmMsg, true);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = pStmMsg->SetSize(ULongLongToULargeInt(cbHeader));
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = pStmMsg->Write(pbBegin, cbHeader, &cbWritten);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      if ( cbWritten < cbHeader )
      {
         hr = STG_E_CANTSAVE;
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = pStmMsg->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = MimeOleCreatePropertySet(NULL, &pPropSet);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }         

      /* Load it up! */
      hr = pPropSet->Load(pStmMsg);
      if ( FAILED(hr) )
      {
         /* Damnit! */
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

#endif /* _LOADMULTIPART */

      /* These need to be removed.. */
      pPropSet->DeleteProp(PIDTOSTR(PID_HDR_LINES));
      pPropSet->DeleteProp(PIDTOSTR(PID_HDR_MIMEVER));
      
      hr = pStmMsg->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = pStmMsg->SetSize(ULongLongToULargeInt(0));
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
      
      hr = pPropSet->Save(pStmMsg, FALSE);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      hr = _LoadMessageStream(pMimeMessage, pStmMsg, FALSE);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      pPropSet->Release();
      pPropSet = NULL;

      pStmMsg->Release();
      pStmMsg = NULL;

#ifdef _NOCLEANTREEONSAVE
      _SetMessageOption(pMimeMessage, OID_CLEANUP_TREE_ON_SAVE, VT_BOOL, (void*)FALSE);
#endif 
      
      /* Determine the codepage to use for any text conversions */
      HCHARSET     hCharset = NULL;
      INETCSETINFO csInfo   = {0};

      hr = pMimeMessage->GetCharset(&hCharset);
      if ( SUCCEEDED(hr) )
      {
         hr = MimeOleGetCharsetInfo(hCharset, &csInfo);
      }

      if ( 0 == csInfo.cpiInternet )
      {
         csInfo.cpiInternet = CP_USASCII;
      }

      /* Get the subject header and parse it for part/total info */
      opSubj.vt = VT_LPSTR;
      hr = pMimeMessage->GetBodyProp(HBODY_ROOT, 
                                     PIDTOSTR(PID_HDR_SUBJECT), 
                                     0, 
                                     &opSubj);

      if ( SUCCEEDED(hr) && (NULL != opSubj.pszVal) )
      {
         size_t cbLen = strlen(opSubj.pszVal);
         /* Don't care if this succeeds or not */
         DecoderGetSubjectParts(opSubj.pszVal, 
                                cbLen, 
                                static_cast<PENCODEINFO>(pRecLink));
      }

      /* Now loop over the records in the list and do the magic */
      LPCBYTE pbText = NULL;
      HBODY   hBody  = NULL;

      while ( NULL != pLink )
      {
         pRecLink = static_cast<RECORDINFO*>(pLink);

         /* Ok this isn't so magical. During this loop we need to handle any
          * text bodies that were stuck between multiple yEnc bodies in the
          * message. It's a pretty straight forward procedure. */
         if ( (NULL != pbText) && ((pRecLink->DecoderBegin - pbText) > 0) )
         {
            /* The text needs a stream... */
            hr = _CreateMimeStream(&pStmMsg, true);
            if ( FAILED(hr) )
            {
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            ULONG cbText = (ULONG)(pRecLink->DecoderBegin - pbText);

            hr = pStmMsg->SetSize(ULongLongToULargeInt(cbText));
            if ( FAILED(hr) )
            {
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            cbWritten = 0;
            hr = pStmMsg->Write(pbText, 
                                cbText, 
                                &cbWritten);

            if ( FAILED(hr) || (cbWritten < cbText) )
            {
               hr = STG_E_CANTSAVE;
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            hr = _AttachStream(pMimeMessage, 
                               pStmMsg, 
                               IET_8BIT,
                               STR_MIME_TEXT_PLAIN, 
                               NULL, 
                               0, 
                               0, 
                               0,
                               &hBody);

            if ( FAILED(hr) )
            {
               /* Failure */
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            /* The pStmMsg variable will be reused so it needs to be cleared out */
            pStmMsg->Release();
            pStmMsg = NULL;
         }

         /* Initialize the decoder stream */
         hr = _CreateMimeStream(&pStmMsg, false);
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }
         
         BOOL   bRet    = FALSE;
         size_t cchFile = 0;
         WCHAR  chFile[MAX_PATH];
         *chFile = L'\0';
            
         /* Set the size to what is reported in the yEnc header */
         size_t cbEncoded = DecoderGetPartSize(static_cast<PENCODEINFO>(pRecLink));
         ATLASSERT(cbEncoded > 0);

         /* With the yEnc format, the decoded size can never be more than 2x the actual
          * payload size because in the worst case, every byte is escaped. We'll do some
          * minimal validation here to verify that.
          *
          * Also note that cbData is from the encoded payload to the end of the =yend
          * line or buffer.
          */
         ULONG cbData = (ULONG)(pRecLink->DecoderEnd - static_cast<PENCODEINFO>(pRecLink)->DataStart);         
         if ( (cbEncoded / 2) > cbData )
         {
            /* Build a filename with the invalid size */
            if ( DecoderGetFileName(chFile,
                                    _countof(chFile),
                                    csInfo.cpiInternet,
                                    static_cast<PENCODEINFO>(pRecLink),
                                    NULL, 
                                    NULL, 
                                    NULL, 
                                    NULL) )
            {
               /* Append the default yEnc extension */
               hr = StringCchCatW(chFile, _countof(chFile), _ltext(OEYENC_YENCFILEEXT));
            }

            /* Use the default name if a proper one could not be constructed */
            if ( FAILED(hr) )
            {
               StringCchCopyW(chFile, _countof(chFile), _ltext(OEYENC_UNKOWNFILE));
            }

            /* Force the payload to be attached as plain text */
            hr = YDEC_E_INVALID_VALUE;            
         }
         else
         {
            /* Force the stream size to be at least 64KB */
            cbEncoded = max(cbEncoded, 64 * 1024);

            hr = pStmMsg->SetSize(ULongLongToULargeInt(cbEncoded));
            if ( FAILED(hr) )
            {
               /* What the hell... */
               break;
            }

            /* Get an address and allocation size for the decoder stream */
            hr = pStmMsg->QueryInterface(IID_IMemoryStream, (void**)&pMemMsg);
            ATLASSERT(SUCCEEDED(hr));

            ULONG cbMsg = 0;
            PBYTE pbMsg = NULL;            
            /* This will fail if there isn't enough memory to commit the size */
            hr = pMemMsg->GetMemoryInfo((LPVOID*)&pbMsg, &cbMsg);
            if ( FAILED(hr) )
            {
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            /* Decode the yEnc body into the message stream */
            bRet = DecoderProcessBuffer(pRecLink->DataStart, 
                                        cbData, 
                                        static_cast<PENCODEINFO>(pRecLink), 
                                        pbMsg, 
                                        cbMsg);

            if ( !bRet )
            {
               hr = CoStatusFromWin32(GetLastError());
               /* If this throws then GetLastError() returned 0. Fix the DecoderXxx routine */
               ATLASSERT(FAILED(hr));
            }

            /* We want to handle failure for a few events that are about to happen, so
             * we plow through until we hit the end, only doing things of importance
             * when the previous event succeeded */
         #ifdef _NOPARTNUMBERS
            pRecLink->EncodeFlags &= ~(EIF_PART_ENCODED|EIF_TOTAL_ENCODED);
         #endif

            if ( SUCCEEDED(hr) )
            {
               /* Update the real size of the attachment stream */
               cbData = (ULONG)pRecLink->SizeActual;
               
            #ifdef _NOPARTSIZE
               /* Force the size to be what the decoder will expect it to be */
               pRecLink->SizeActual = DecoderGetPartSize(static_cast<PENCODEINFO>(pRecLink));
            #endif /* _NOPARTSIZE */

               /* Build a file name for this attachment */
               CRC32 iCrc32 = 0;
               bRet = DecoderGetFileName(chFile, 
                                         _countof(chFile), 
                                         csInfo.cpiInternet, 
                                         static_cast<PENCODEINFO>(pRecLink), 
                                         pbMsg,
                                         &cchFile,
                                         &iCrc32,
                                         &__CRC32TABLE);

               if ( !bRet )
               {
                  hr = CoStatusFromWin32(GetLastError());
                  /* If this throws then GetLastError() returned 0. Fix the DecoderGetFileName routine */
                  ATLASSERT(FAILED(hr));
               }

               /* If we couldn't get a filename, give it a default one */
               if ( FAILED(hr) )
               {
                  hr = StringCchCopyW(chFile, _countof(chFile), _ltext(OEYENC_UNKOWNFILE));
               }

               if ( _GetRegSetting(HKEY_CURRENT_USER,
                                   _T(OEYENC_REG_SUBKEY),
                                   _T(OEYENC_REG_REPORT),
                                   &bAttachReport,
                                   sizeof(BOOL)) && bAttachReport )
               {
                  _CreateDecoderReport(pMimeMessage, 
                                       pbMsg, 
                                       cbData, 
                                       chFile, 
                                       pRecLink->PartEncoded, 
                                       pRecLink->TotalEncoded, 
                                       iCrc32, 
                                       &hBody);
               }
            }

            /* Get the file extension from the name the decoder built and ask MIMEOLE if it
             * can come up with a suitable content-type for it. */
            if ( SUCCEEDED(hr) )
            {
               /* Terminate the file name at the end of the reported file extension. This is
                * necessary because DecoderGetFileName() may add part info which will give
                * a different content type than the file's actual extension */
               WCHAR chTerm = L'\0';
               if ( cchFile > 0 )
               {
                  chTerm = chFile[cchFile];
                  chFile[cchFile] = L'\0';
               }

               /* We use a homegrown version of PathFindExtension because the
                * one from SHLWAPI can't handle multiple . (dots) in the filename */
               LPWSTR pwszExt = _PathFindExtensionW(chFile);

               if ( (NULL != pwszExt) && (L'\0' != *pwszExt) )
               {
                  /* MIMEOLE won't do widechar matches so convert just the extension to its
                   * multibyte version */
                  CHAR chExt[MAX_PATH];               
                  int  cchExt = WideCharToMultiByte(csInfo.cpiInternet, 
                                                   0, 
                                                   pwszExt, 
                                                   -1, 
                                                   chExt, 
                                                   (int)sizeof(chExt), 
                                                   NULL, 
                                                   NULL);

                  if ( cchExt > 0 )
                  {
                     /* Now ask MIMEOLE if it has a content-type for us. If this fails a default one will 
                      * be used. Failure is indicated by pszCntType being equal to NULL */
                     MimeOleGetExtContentType(chExt, &pszCntType);

                  #ifdef _DEBUG
                     if ( !pszCntType )
                     {
                        ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("!Failed to acquire content-type for extension \"%hs\"\n"), pMimeMessage, chExt);
                     }
                  #endif
                  }
               }

               if ( cchFile > 0 )
               {
                  chFile[cchFile] = chTerm;
               }
            }
         }/*END-OF: if ( (cbEncoded / 2) > cbData ) */

         /* Ok, now the events are done. If any critical piece failed we're going
          * to write the _encoded_ data to the attachment stream and give it to
          * the message, otherwise we're attaching the _decoded_ stream in all its glory */
         LPCSTR  pszType = pszCntType;
         LPCWSTR pszName = chFile;
         USHORT  iPart   = (FlagOn(pRecLink->EncodeFlags, EIF_PART_ENCODED)  ? pRecLink->PartEncoded  : 0);
         USHORT  iTotal  = (FlagOn(pRecLink->EncodeFlags, EIF_TOTAL_ENCODED) ? pRecLink->TotalEncoded : 0);

      #ifdef _LOADMULTIPART
         if ( iPart > 1 )
         {
            pszName = NULL;
            pszType = STR_MIME_MSG_PART;
         }
      #endif /* _LOADMULTIPART */

         if ( FAILED(hr) )
         {
            /* Copy the data that couldn't be decoded into the message stream so 
             * it can be attached as text. This includes the full yenc payload,
             * from =ybegin[2] to the end of the =yend line or buffer. */
            cbData = (ULONG)(pRecLink->DecoderEnd - pRecLink->DecoderBegin);

            cbWritten = 0;
            hr = pStmMsg->Write(pRecLink->DecoderBegin, 
                                (ULONG)cbData, 
                                &cbWritten);

            if ( FAILED(hr) || (cbWritten < cbData) )
            {
               hr = STG_E_CANTSAVE;
               ATLASSERT(SUCCEEDED(hr));
               __leave;
            }

            /* Reset the pieces that are now inapplicable */
            pszType = NULL;
         }

         /* Reset the attachment stream size to the determined size and give it
          * to the message. Again we will plow through some steps and handle failure
          * after they're all done */
         hr = pStmMsg->SetSize(ULongLongToULargeInt(cbData));
         if ( SUCCEEDED(hr) )
         {
            hr = _AttachStream(pMimeMessage, 
                               pStmMsg, 
                               IET_BINARY,
                               NULL != pszType ? pszType : STR_MIME_APPL_STREAM, 
                               pszName, 
                               iPart, 
                               iTotal,
                               cbData,
                               &hBody);
         }

         /* Serialize the yEnc attributes so they can be added to the X-OeyEnc header. For
          * now all the header is used for is to determine if the message was previously 
          * parsed by this routine. The idea for the future is to keep the CRC, total size
          * and etc for more stringent validation. Also, this is only done if the message
          * was actually decoded, in which case bRet != FALSE */
         if ( bRet && SUCCEEDED(hr) )
         {
            WCHAR chHdr[MV_MAXLINE];

            bRet = DecoderSerializeEncodeInfo(chHdr, 
                                              _countof(chHdr), 
                                              csInfo.cpiInternet, 
                                              EIF_SIZE_ENCODED|EIF_TOTAL_ENCODED|EIF_CRC_ENCODED|EIF_NAME_ENCODED, 
                                              static_cast<PENCODEINFO>(pRecLink));
            if ( !bRet )
            {
               hr = CoStatusFromWin32(GetLastError());
               /* If this throws then GetLastError() returned 0. Fix the DecoderSerializeEncodeInfo routine */
               ATLASSERT(FAILED(hr));
            }

            if ( SUCCEEDED(hr) )
            {
               /* Add the OeyEnc header, but skip the first space */
               ATLASSERT(' ' == *chHdr);
               hr = _AppendBodyHeader(pMimeMessage, HBODY_ROOT, OEYENC_HEADER, chHdr + sizeof(' '));
            }
         }

         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         /* Cleanup resources that will be reused in the next iteration... */
         CoTaskMemFree(pszCntType);
         pszCntType = NULL;

         pStmMsg->Release();
         pStmMsg = NULL;

         if ( NULL != pMemMsg )
         {
            pMemMsg->Release();
            pMemMsg = NULL;
         }

         /* Save the offset of what might be the start of a text body */
         pbText = pRecLink->DecoderEnd;
         
         pLink = pLink->Next;
      }/*END-OF: while ( NULL != pLink ) */

      /* The list has been exhuasted but there may still be text
       * which needs to be added to the message */
      if ( (NULL != pbText) && (pbText < pbEnd) )
      {
         hr = _CreateMimeStream(&pStmMsg, true);
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         hr = pStmMsg->SetSize(ULongLongToULargeInt(pbEnd - pbText));
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         cbWritten = 0;
         hr = pStmMsg->Write(pbText, 
                             (ULONG)(pbEnd - pbText), 
                             &cbWritten);

         if ( FAILED(hr) || (cbWritten < (ULONG)(pbEnd - pbText)) )
         {
            hr = STG_E_CANTSAVE;
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         hr = _AttachStream(pMimeMessage, 
                            pStmMsg, 
                            IET_8BIT,
                            STR_MIME_TEXT_PLAIN, 
                            NULL, 
                            0, 
                            0, 
                            0,
                            &hBody);

         if ( FAILED(hr) )
         {
            /* Failure */
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         pStmMsg->Release();
         pStmMsg = NULL;
      }

   #ifdef _HANDSOFF
      pMimeMessage->HandsOffStorage();
   #endif /* _HANDSOFF */
      pMimeMessage->Commit(COMMIT_ONLYIFDIRTY|COMMIT_REUSESTORAGE);
   }
   #pragma warning( suppress : 6320 )
   __except( DBG_EXCEPTION_EXECUTE_HANDLER )
   {
      hr = HRESULT_FROM_NT(GetExceptionCode());
   }

   delete pRecord;
   PropVariantClear(&opSubj);
   CoTaskMemFree(pszCntType);
   
   pLink = xHead.Next;   
   while ( NULL != pLink )
   {
      pRecord = static_cast<RECORDINFO*>(pLink);
      pLink = pLink->Next;
      delete pRecord;         
   }

   if ( NULL != pStmMsg )
   {
      pStmMsg->Release();
   }

   if ( NULL != pMemText )
   {
      pMemText->Release();
   }

   return ( hr );
}

HRESULT CMimeMessage::_AttachStream( IMimeMessage* pMimeMessage, IStream* pStream, ENCODINGTYPE ietEncoding, LPCSTR pszCntType, LPCWSTR pszFileName, USHORT iPart, USHORT iTotal, ULONG cbSize, LPHBODY phBody ) throw()
{
   ATLASSERT(NULL != pMimeMessage && NULL != pStream);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMimeMessage);
   
   (*phBody) = NULL;

   HRESULT    hr        = E_FAIL;
   IMimeBody* pMimeBody = NULL;
#ifdef _COPYATTACH
   IStream*   pStmCopy  = NULL;
#endif /* _COPYATTACH */

   __try
   {
   #ifdef _COPYATTACH
      {
         //hr = MimeOleCreateVirtualStream(&pStmCopy);
         hr = _CreateMimeStream(&pStmCopy, false);
         if ( FAILED(hr) )
         {
            __leave;
         }

         hr = CopyStream(pStream, pStmCopy, NULL);
         if ( FAILED(hr) )
         {
            __leave;
         }
         
         pStream = pStmCopy;
      }
   #endif /* _COPYATTACH */

      hr = pStream->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Attach the object, whatever it is */
      HBODY hBody = NULL;

   #ifdef _LOADMULTIPART
      /* Find the first multipart message or create one if it doesn't exist */
      FINDBODY fbInfo = {(LPSTR)STR_CNT_MULTIPART, NULL, 0};

      hr = pMimeMessage->FindFirst(&fbInfo, &hBody);
      if ( FAILED(hr) )
      {
         hr = pMimeMessage->ToMultipart(HBODY_ROOT, STR_SUB_MIXED, &hBody);
      }
      else 
      {
         hr = pMimeMessage->InsertBody(IBL_LAST, hBody, &hBody);   
      }
   #else /* _LOADMULTIPART */
      hr = pMimeMessage->AttachObject(IID_IStream, (void*)pStream, &hBody);
   #endif /* _LOADMULTIPART */
      
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
      
      (*phBody) = hBody;

      hr = pMimeMessage->BindToObject(hBody, IID_IMimeBody, (void**)&pMimeBody);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      
   #ifdef _LOADMULTIPART
      hr = pMimeBody->SetData(IET_BINARY, STR_CNT_APPLICATION, STR_SUB_OCTETSTREAM, IID_IStream, (LPVOID)pStream);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }
   #endif /* _LOADMULTIPART */

      /* Set the encoding for the new body */
      hr = pMimeBody->SetCurrentEncoding(ietEncoding);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      /* Set the attachment properties */
      CHAR chBuf[MAX_PATH];
      chBuf[0] = '\0';

      hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(PID_HDR_CNTTYPE), 0, VT_LPSTR, (void*)pszCntType);

   #ifndef _NOPARTSIZE
      if ( SUCCEEDED(hr) && (cbSize > 0) )
      {
         if ( SUCCEEDED(StringCchPrintfA(chBuf, _countof(chBuf), "%u", cbSize)) )
         {
            hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(STR_PAR_SIZE), 0, VT_LPSTR, (void*)chBuf);
         }
      }
   #endif /* _NOPARTSIZE */

      if ( SUCCEEDED(hr) && (NULL != pszFileName) )
      {
         hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(STR_PAR_FILENAME), 0, VT_LPWSTR, (void*)pszFileName);

         if ( SUCCEEDED(hr) )
         {
            hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(PID_ATT_FILENAME), 0, VT_LPWSTR, (void*)pszFileName);
         }
      }

   #ifndef _NOPARTNUMBERS
      if ( SUCCEEDED(hr) && (iPart > 0) )
      {
         if ( SUCCEEDED(StringCchPrintfA(chBuf, _countof(chBuf), "%02u", iPart)) )
         {
            hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(STR_PAR_NUMBER), 0, VT_LPSTR, (void*)chBuf);
         }
      }

      if ( SUCCEEDED(hr) && (iTotal > 0) )
      {
         if ( SUCCEEDED(StringCchPrintfA(chBuf, _countof(chBuf), "%u", iTotal)) )
         {
            hr = _SetBodyProp(pMimeMessage, hBody, PIDTOSTR(STR_PAR_TOTAL), 0, VT_LPSTR, (void*)chBuf);
         }
      }
   #endif /* _NOPARTNUMBERS */
   }
   __finally
   {
      if ( NULL != pMimeBody )
      {
         pMimeBody->Release();
      }

   #ifdef _COPYATTACH
      if ( NULL != pStmCopy )
      {
         pStmCopy->Release();
      }
   #endif /* _COPYATTACH */
   }

   return ( hr );
}

HRESULT CMimeMessage::_CombineRelatedAttachments( IMimeMessage* pMsgSource, IMimeMessage* pMsgTarget ) throw()
{
   ATLASSERT(NULL != pMsgSource && NULL != pMsgTarget);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMsgSource);
   
   HRESULT     hr         = E_FAIL;
   LPHBODY     rgAttach   = NULL;
   IMimeBody*  pMimeBody  = NULL;
   IStream*    pStmAttach = NULL;
   IStream*    pStmText   = NULL;
   IStream*    pStmHtml   = NULL;
   PROPVARIANT opName     = {0};
   LPSTR       pszCntType = NULL;
   ULONG       idx;
   ULONG       jdx;
   __try
   {
      ULONG cAttach = 0;
      hr = pMsgSource->GetAttachments(&cAttach, &rgAttach);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("\t%p!") _T(__FUNCTION__) _T("!%u attachments\n"), pMsgSource, cAttach);

      /* First find and force all text/plain and text/html bodies to be inlined. This is because OE 
       * adds tiny text bodies to each attachment and gives them unique names. Removing them from the 
       * list will make the code below easier */
      for ( idx = 0; idx < cAttach; idx++ )
      {
         HBODY hBody = rgAttach[idx];

         hr = pMsgSource->BindToObject(hBody, IID_IMimeBody, (void**)&pMimeBody);
         if ( FAILED(hr) )
         {
            continue;
         }
            
         /* Never inline the decoder reports, which are text/plain */
         if ( S_OK == pMimeBody->QueryProp(PIDTOSTR(PID_ATT_FILENAME), OEYENC_REPORTFILE, FALSE, FALSE) )
         {
            hr = E_FAIL;
         }
         else
         {
            if ( S_OK == pMimeBody->IsContentType(STR_CNT_TEXT, STR_SUB_PLAIN) )
            {
               hr = (!pStmText ? _CreateMimeStream(&pStmText, true) : S_OK);
               if ( SUCCEEDED(hr) )
               {
                  hr = pMimeBody->GetDataHere(IET_UNICODE, pStmText);
               }
            }
            else if ( S_OK == pMimeBody->IsContentType(STR_CNT_TEXT, STR_SUB_HTML) )
            {
               hr = (!pStmHtml ? _CreateMimeStream(&pStmHtml, true) : S_OK);
               if ( SUCCEEDED(hr) )
               {
                  hr = pMimeBody->GetDataHere(IET_UNICODE, pStmHtml);
               }
            }
            else
            {
               hr = E_FAIL;
            }
         }
      
         pMimeBody->Release();
         pMimeBody = NULL;

         if ( SUCCEEDED(hr) )
         {
            rgAttach[idx] = NULL;
         }
      }
      
      /* Now get the text bodies of the source message. We support both plain and HTML
       * text bodies in the same message */
      HBODY hBody = NULL;
      
      if ( NULL != pStmText )
      {
         hr = pStmText->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
         if ( SUCCEEDED(hr) )
         {
            hr = _AppendTextStream(pMsgTarget, pStmText, IET_UNICODE, TXT_PLAIN, &hBody);
         }
      }

      if ( NULL != pStmHtml )
      {
         hr = pStmHtml->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
         if ( SUCCEEDED(hr) )
         {
            hr = _AppendTextStream(pMsgTarget, pStmHtml, IET_UNICODE, TXT_HTML, &hBody);
         }
      }

      /* This is the main 'combine' body of code. Basically it goes down like this...
         1) find a non-NULL body and get its filename
         2) scan the rest of the array for non-NULL bodies matching the filename
         and add their estimated size to a running total
         3) create a new stream of the size specified by the running total and rescan 
         the array for bodies matching the filename, however this time move the body 
         data to the new stream

         So every unique filename is a group which causes the remaining list to be
         searched for matching names. A match is any body whose name contains the
         current filename being searched for. 
         
         So for instance, the files below are joined as follows...
            file.ext
            file.ext.1
            file.ext.2
         joined as...
            file.ext

         and...
            file.Foo
            file.foo.4.2
            file.fOo.2
            file.foOO.1
            file.bar
            file.bar.1
            file.bar.2
         joined as...
            file.Foo
            file.bar

         however, the following are not combined...
            file.Ext
            file1.eXt
            file2.exT

         We ignore ordering because OE has a dialog for doing that, and we ignore
         the case because on Windows everything is case-insensitive (and because
         I don't care about some addon or file system (Services for Unix) that is 
         case-sensitive). 
       */

      /* STEP-1 */
      for ( idx = 0; idx < cAttach; idx++ )
      {
         hBody = rgAttach[idx];
         if ( !hBody )
         {
            continue;
         }

         /* Get the filename for this body */
         opName.vt = VT_LPSTR;
         hr = pMsgSource->GetBodyProp(hBody, PIDTOSTR(PID_ATT_FILENAME), 0, &opName);
         if ( FAILED(hr) )
         {
            PropVariantClear(&opName);
            /* Oh well... keep going anyway */
            continue;
         }

         /* STEP-2 */

         /* First find all the bodies which belong to this one and calculate the
          * estimated size of the combined stream. This will include the current
          * body identified by rgAttach[idx] */
         ULONG cbTotal = 0;

         for ( jdx = idx; jdx < cAttach; jdx++ )
         {
            hBody = rgAttach[jdx];
            if ( !hBody )
            {
               continue;
            }

            hr = pMsgSource->BindToObject(hBody, IID_IMimeBody, (void**)&pMimeBody);
            if ( FAILED(hr) )
            {
               /* Oh well... keep going anyway */
               continue;
            }

            hr = pMsgSource->QueryBodyProp(hBody, PIDTOSTR(PID_ATT_FILENAME), opName.pszVal, TRUE, FALSE);
            if ( S_OK == hr )
            {
               ULONG cbSize = 0;
               hr = pMimeBody->GetEstimatedSize(IET_BINARY, &cbSize);
               if ( SUCCEEDED(hr) )
               {
                  /* Check for overflow... */
                  ULONG cbTemp = cbTotal + cbSize;
                  if ( cbTemp > cbTotal )
                  {
                     cbTotal = cbTemp;
                  }
               }
            }

            pMimeBody->Release();
            pMimeBody = NULL;
         }/*END-OF: for ( ULONG jdx = ... */

         /* STEP-3 */

         /* Now allocate a stream of the estimated size for the combined attachment */
      #ifdef _NOCOMBINEMEMSTREAM
         hr = MimeOleCreateVirtualStream(&pStmAttach);
      #else
         hr = _CreateMimeStream(&pStmAttach, false);
      #endif
         if ( FAILED(hr) ) 
         {
            /* Must be out of memory... */
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }

         hr = pStmAttach->SetSize(ULongLongToULargeInt(cbTotal));
         if ( FAILED(hr) ) 
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }
         
         /* Loop through the list again and add each related body to the new attachment. This
          * will include the current body identified by rgAttach[idx] */
         for ( jdx = idx; jdx < cAttach; jdx++ )
         {
            hBody = rgAttach[jdx];
            if ( !hBody )
            {
               continue;
            }

            hr = pMsgSource->BindToObject(hBody, IID_IMimeBody, (void**)&pMimeBody);
            if ( FAILED(hr) )
            {
               /* Oh well... keep going anyway */
               continue;
            }

            hr = pMsgSource->QueryBodyProp(hBody, PIDTOSTR(PID_ATT_FILENAME), opName.pszVal, TRUE, FALSE);
            if ( S_OK == hr )
            {
               /* Clear this entry in the list so it can be ignored */
               rgAttach[jdx] = NULL;

               hr = pMimeBody->GetDataHere(IET_BINARY, pStmAttach);
               if ( SUCCEEDED(hr) )
               {
                  /* Free up some memory */                  
                  hr = pMimeBody->EmptyData();
               }
            }

            pMimeBody->Release();
            pMimeBody = NULL;
         }/*END-OF: for ( ULONG jdx = ... */

         /* The estimated size may have been larger than what was actually written to the
          * new attachment stream so set its size to the real thing */
         ULARGE_INTEGER cbSize = {0};
         hr = pStmAttach->Seek(LongLongToLargeInt(0), SEEK_CUR, &cbSize);
         if ( SUCCEEDED(hr) )
         {
            hr = pStmAttach->SetSize(cbSize);
            if ( SUCCEEDED(hr) )
            {
            #ifdef _DUMPSTREAMONLOAD
               DumpStreamToFile(pStmAttach, _T("C:\\Temp\\CMimeMessage_CombineRelatedAttachments"));
            #endif

               hr = pMsgTarget->AttachObject(IID_IStream, (void*)pStmAttach, &hBody);
               if ( SUCCEEDED(hr) )
               {
                  LPSTR pszExt = _PathFindExtensionA(opName.pszVal);
                  if ( (NULL != pszExt) && ('\0' != *pszExt) )
                  {
                     MimeOleGetExtContentType(pszExt, &pszCntType);
                  }

                  /* Set the attachment properties */
                  hr = _SetBodyProp(pMsgTarget, hBody, PIDTOSTR(PID_HDR_CNTTYPE), 0, VT_LPSTR, (void*)(NULL != pszCntType ? pszCntType : STR_MIME_APPL_STREAM));
            
                  if ( SUCCEEDED(hr) )
                  {
                     hr = _SetBodyProp(pMsgTarget, hBody, PIDTOSTR(PID_ATT_FILENAME), 0, VT_LPSTR, (void*)opName.pszVal);
                  }
               }
            }
         }

         pStmAttach->Release();
         pStmAttach = NULL;

         CoTaskMemFree(pszCntType);
         pszCntType = NULL;

         PropVariantClear(&opName);
      }/*END-OF: for ( ULONG idx = ... */

      /* Everything is done... */
   #ifdef _HANDSOFF
      pMsgTarget->HandsOffStorage();
   #endif
      pMsgTarget->Commit(COMMIT_ONLYIFDIRTY|COMMIT_REUSESTORAGE);
   }
   #pragma warning( suppress : 6320 )
   __except( DBG_EXCEPTION_EXECUTE_HANDLER )
   {
      hr = HRESULT_FROM_NT(GetExceptionCode());
   }

   if ( NULL != pStmHtml )
   {
      pStmHtml->Release();
   }

   if ( NULL != pStmText )
   {
      pStmText->Release();
   }

   if ( NULL != pStmAttach )
   {
      pStmAttach->Release();
   }

   if ( NULL != pMimeBody )
   {
      pMimeBody->Release();
   }
   
   if ( NULL != rgAttach )
   {
      CoTaskMemFree(rgAttach);
   }

   PropVariantClear(&opName);

   if ( NULL != pszCntType )
   {
      CoTaskMemFree(pszCntType);
   }

   return ( hr );
}

HRESULT CMimeMessage::_AppendBodyHeader( IMimeMessage* pMimeMessage, HBODY hBody, LPCSTR pszName, LPCWSTR pszValue ) throw()
{
   ATLASSERT(NULL != pMimeMessage);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMimeMessage);
   
   HRESULT           hr       = E_FAIL;
   IMimePropertySet* pPropSet = NULL;

   __try
   {
      hr = pMimeMessage->BindToObject(hBody, IID_IMimePropertySet, (void**)&pPropSet);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

      PROPVARIANT prop = {0};
      prop.vt      = VT_LPWSTR;
      prop.pwszVal = (LPWSTR)pszValue;
      hr = pPropSet->AppendProp(pszName, PDF_HEADERFORMAT|PDF_VECTOR, &prop);
   }
   __finally
   {
      if ( NULL != pPropSet )
      {
         pPropSet->Release();
      }
   }

   return ( hr );
}

HRESULT CMimeMessage::_LoadMessageStream( IMimeMessage* pMimeMessage, IStream* pStream, BOOL bHeadersOnly ) throw()
{
   ATLASSERT(NULL != pMimeMessage && NULL != pStream);
   ATLTRACE2(atlTraceCOM, eTraceCOMLevel, _T("%p!") _T(__FUNCTION__) _T("\n"), pMimeMessage);
   
   HRESULT           hr       = E_FAIL;
   IMimePropertySet* pPropSet = NULL;

   __try
   {
      hr = pStream->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      ATLASSERT(SUCCEEDED(hr));
      if ( FAILED(hr) )
      {
         __leave;
      }

      if ( bHeadersOnly )
      {
         hr = pMimeMessage->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (void**)&pPropSet);
         ATLASSERT(SUCCEEDED(hr));
         if ( FAILED(hr) )
         {
            __leave;
         }

         hr = pPropSet->Load(pStream);
         ATLASSERT(SUCCEEDED(hr));
         if ( FAILED(hr) )
         {
            __leave;
         }
      }
      else
      {
         hr = pMimeMessage->Load(pStream);
         ATLASSERT(SUCCEEDED(hr));
      }

   #ifdef _HANDSOFF
      pMimeMessage->HandsOffStorage();
   #endif
   }
   __finally
   {
      if ( NULL != pPropSet )
      {
         pPropSet->Release();
      }
   }

   return ( hr );
}

HRESULT CMimeMessage::_CreateDecoderReport( IMimeMessage* pMimeMessage, LPCBYTE pbData, ULONG cbData, LPCWSTR pwszFileName, USHORT /*iPart*/, USHORT /*iTotal*/, CRC32 iCrc, LPHBODY phBody ) throw()
{
   HRESULT     hr      = S_OK;      
   IStream*    pStream = NULL;
   IStream*    pStmBody = NULL;
   HCRYPTPROV  hProv   = NULL;
   HCRYPTHASH  hHash   = NULL;
   LPBYTE      pbHash  = NULL;

   struct ALGINFO
   {
      ALG_ID  AlgID;
      LPCWSTR AlgName;
   };
   
   enum { cbAlgName = 8 * sizeof(WCHAR) };

   static ALGINFO rgAlgInfo[] = 
   {
      {CALG_MD5,     L"MD5     "},
      {CALG_SHA1,    L"SHA1    "}
   };

   __try
   {
      BOOL bRet = CryptAcquireContext(&hProv, 
                                      NULL, 
                                      NULL, 
                                      PROV_DSS, 
                                      CRYPT_VERIFYCONTEXT|CRYPT_SILENT);

      if ( !bRet )
      {
         hr = CoStatusFromWin32(GetLastError());
         __leave;
      }

      hr = _CreateMimeStream(&pStream, true);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

#ifdef _TEXTBODYREPORTS
      hr = pMimeMessage->GetTextBody(TXT_PLAIN, IET_UNICODE, &pStmBody, phBody);
      if ( SUCCEEDED(hr) )
      {
         hr = CopyStream(pStmBody, pStream, NULL);
         if ( FAILED(hr) )
         {
            ATLASSERT(SUCCEEDED(hr));
            __leave;
         }
      }
#endif /* _TEXTBODYREPORTS */

      const WCHAR chCrLf[]    = {L"\r\n"};
      const CHAR  chDigits[]  = {"0123456789ABCDEF"};
      const WCHAR chHeader[]  = {L"\r\n---------------- OeyEnc Decoder Report ----------------\r\n"};
      const WCHAR chFooter[]  = {L"-------------------------------------------------------\r\n"};
      
      ULONG cbWrite     = 0;
      ULONG cbWritten   = 0;
      ULONG cbHash      = 0;

      cbWrite = sizeof(chHeader) - sizeof(WCHAR);
      hr = pStream->Write(chHeader, cbWrite, &cbWritten);
      if ( FAILED(hr) || (cbWritten < cbWrite) )
      {
         hr = STG_E_CANTSAVE;
         __leave;
      }

      if ( NULL != pwszFileName )
      {
         LPWSTR pwszEnd = NULL;
         WCHAR  chFile[MAX_PATH+8];

         if ( SUCCEEDED(StringCchPrintfExW(chFile, _countof(chFile), &pwszEnd, NULL, 0, L"Name    %s\r\n", pwszFileName)) )
         {
            cbWrite = (ULONG)((ULONG_PTR)pwszEnd - (ULONG_PTR)chFile);
            hr = pStream->Write(chFile, cbWrite, &cbWritten);
            if ( FAILED(hr) || (cbWritten < cbWrite) )
            {
               hr = STG_E_CANTSAVE;
               __leave;
            }
         }
      }

      if ( 0 != iCrc )
      {
         LPWSTR pwszEnd = NULL;
         WCHAR  chCrc32[48];      

         if ( SUCCEEDED(StringCchPrintfExW(chCrc32, _countof(chCrc32), &pwszEnd, NULL, 0, L"CRC32   %0.8lX\r\n", iCrc)) )
         {
            cbWrite = (ULONG)((ULONG_PTR)pwszEnd - (ULONG_PTR)chCrc32);
            hr = pStream->Write(chCrc32, cbWrite, &cbWritten);
            if ( FAILED(hr) || (cbWritten < cbWrite) )
            {
               hr = STG_E_CANTSAVE;
               __leave;
            }
         }
      }

      for ( size_t idx = 0; idx < _countof(rgAlgInfo); idx++ )
      {
         __try
         {
            bRet = CryptCreateHash(hProv, rgAlgInfo[idx].AlgID, NULL, 0, &hHash);
            if ( !bRet )
            {
               __leave;
            }

            bRet = CryptHashData(hHash, pbData, cbData, 0);
            if ( !bRet )
            {
               __leave;
            }

            ULONG cbName = 0;
            bRet = CryptGetHashParam(hHash, HP_HASHVAL, NULL, &cbName, 0);
            if ( !bRet )
            {
               __leave;
            }

            if ( !pbHash || (cbName > cbHash) )
            {
               delete [] pbHash;
               cbHash = cbName * 2;
               pbHash = new BYTE[cbHash];
            }

            if ( !pbHash )
            {
               __leave;
            }

            memset(pbHash, 0, cbHash);

            cbName = cbHash;
            bRet = CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &cbName, 0);
            if ( !bRet )
            {
               __leave;
            }

            hr = pStream->Write(rgAlgInfo[idx].AlgName, cbAlgName, &cbWritten);
            if ( FAILED(hr) || (cbWritten < cbAlgName) )
            {
               hr = STG_E_CANTSAVE;
               __leave;
            }

            for ( ULONG j = 0; j < cbName; j++ )
            {
               WCHAR chBuf[128] = {L'\0'};
               ULONG cch        = 0;

               while ( (cch < (sizeof(chBuf) / sizeof(WCHAR))) && (j < cbName) )
               {
                  chBuf[cch++] = chDigits[pbHash[j] >> 4];
                  chBuf[cch++] = chDigits[pbHash[j++] & 0x0f];
               }

               cch--;

               cbWrite = cch * sizeof(WCHAR);
               hr = pStream->Write(chBuf, cbWrite, &cbWritten);
               if ( FAILED(hr) || (cbWritten < cbWrite) )
               {
                  hr = STG_E_CANTSAVE;
                  __leave;
               }
            }

            cbWrite = sizeof(chCrLf) - sizeof(WCHAR);
            hr = pStream->Write(chCrLf, cbWrite, &cbWritten);
            if ( FAILED(hr) || (cbWritten < cbWrite) )
            {
               hr = STG_E_CANTSAVE;
               __leave;
            }
         }
         __finally
         {
            if ( NULL != hHash )
            {
               CryptDestroyHash(hHash);
               hHash = NULL;
            }
         }
      }/*END-OF: for ( size_t idx = ... */
         
      cbWrite = sizeof(chFooter) - sizeof(WCHAR);
      hr = pStream->Write(chFooter, cbWrite, &cbWritten);
      if ( FAILED(hr) || (cbWritten < cbWrite) )
      {
         hr = STG_E_CANTSAVE;
         __leave;
      }

      hr = pStream->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      if ( FAILED(hr) )
      {
         ATLASSERT(SUCCEEDED(hr));
         __leave;
      }

#ifdef _DUMPSTREAMREPORTS
      DumpStreamToFile(pStream, _T("C:\\Temp\\yEnc\\Report-"), 0);
#endif

#ifdef _TEXTBODYREPORTS
      hr = _AppendTextStream(pMimeMessage, pStream, IET_UNICODE, TXT_PLAIN, phBody);
#else /* _TEXTBODYREPORTS */
      hr = _AttachStream(pMimeMessage, pStream, IET_UNICODE, STR_MIME_TEXT_PLAIN, _ltext( OEYENC_REPORTFILE ), 0, 0, 0, phBody);
#endif /* _TEXTBODYREPORTS */
   }
   __finally
   {
      delete [] pbHash;

      if ( NULL != hHash )
      {
         CryptDestroyHash(hHash);
      }

      if ( NULL != hProv )
      {
         CryptReleaseContext(hProv, 0);
      }

      if ( NULL != pStmBody )
      {
         pStmBody->Release();
      }

      if ( NULL != pStream )
      {
         pStream->Release();
      }
   }

   return ( hr );
}

HRESULT CMimeMessage::_AppendTextStream( IMimeMessage* pMimeMessage, IStream* pStream, ENCODINGTYPE ietEncoding, DWORD dwTxtType, LPHBODY phBody ) throw()
{
   HRESULT  hr       = E_FAIL;
   IStream* pStmBody = NULL;
   IStream* pStmCopy = NULL;

   __try
   {
      hr = pMimeMessage->GetTextBody(dwTxtType, ietEncoding, &pStmBody, phBody);
      if ( FAILED(hr) )
      {
         if ( MIME_E_NO_DATA == hr )
         {
            hr = pMimeMessage->SetTextBody(dwTxtType, ietEncoding, NULL, pStream, phBody);
         }

         __leave;
      }

      hr = _CreateMimeStream(&pStmCopy, true);
      if ( FAILED(hr) )
      {
         __leave;
      }

      hr = CopyStream(pStmBody, pStmCopy, NULL);
      if ( FAILED(hr) )
      {
         __leave;
      }

      hr = CopyStream(pStream, pStmCopy, NULL);
      if ( FAILED(hr) )
      {
         __leave;
      }
         
      hr = pStmCopy->Seek(LongLongToLargeInt(0), SEEK_SET, NULL);
      if ( FAILED(hr) )
      {
         __leave;
      }
      
      hr = pMimeMessage->SetTextBody(dwTxtType, ietEncoding, NULL, pStmCopy, phBody);
   }
   __finally
   {
      if ( NULL != pStmBody )
      {
         pStmBody->Release();
      }

      if ( NULL != pStmCopy )
      {
         pStmCopy->Release();
      }
   }

   return ( hr );
}

HRESULT CMimeMessage::_CreateMimeStream( IStream** ppStream, bool bByteStream )
{
#ifdef _USEVERIFIERHEAP
   bByteStream;
   return ( CreateVerifierStream(ppStream) );
#else /* _USEVERIFIERHEAP */
   if ( bByteStream )
   {
      return ( CreateByteStream(ppStream) );
   }
      
   return ( CreateVirtualStream(ppStream) );
#endif /* _USEVERIFIERHEAP */
}

#ifdef UNICODE
// Restoring predefined unicode macros
#pragma pop_macro("GetProp")
#pragma pop_macro("SetProp")
#endif // UNICODE

//#pragma strict_gs_check(pop)
