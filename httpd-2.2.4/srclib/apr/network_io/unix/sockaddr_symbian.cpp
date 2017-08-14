/* Copyright 2008 Nokia Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <commdb.h>
#include <commdbconnpref.h>
#include <es_sock.h>
#include <in_sock.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>


namespace
{
    int Copy_h_name(const char*     apName,
                    struct hostent* apReturn,
                    char*&          apBuffer,
                    size_t&         aBufLen)
        {
        int
            rc = 0;
        
        char
            *pEnd = apBuffer + aBufLen;
        int
            size = strlen(apName) + 1;
        
        if (apBuffer + size < pEnd)
            {
            apReturn->h_name = apBuffer;
            
            strcpy(apBuffer, apName);
            
            apBuffer += size;
            aBufLen  -= size;
            }
        else
            rc = -1;

        return rc;
        }
}


namespace
{

    int GetHostByNameL(const TNameRecord& aRecord,
                       struct hostent*    apReturn,
                       char*              apBuffer,
                       size_t             aBufLen,
                       struct hostent**   appResult,
                       int*               apErrno)
        {
        int
            rc = 0;
           
        memset(apReturn, 0, sizeof(*apReturn));

        HBufC8
            *pName = HBufC8::NewLC(aRecord.iName.Length() + 1);

        pName->Des().Copy(aRecord.iName);

        rc = Copy_h_name(reinterpret_cast<const char*>(pName->Des().PtrZ()),
                         apReturn, apBuffer, aBufLen);

        if (rc == 0)
            {
            apReturn->h_addrtype = AF_INET;
            apReturn->h_length   = sizeof(in_addr);
            
            char
                *pEnd = apBuffer + aBufLen;
            int
                size = 2 * sizeof(char*);

            if (apBuffer + size < pEnd)
                {
                apReturn->h_addr_list = reinterpret_cast<char**>(apBuffer);
                
                apBuffer += size;
                aBufLen  -= size;

                size = apReturn->h_length;

                if (apBuffer + size < pEnd)
                    {
                    apReturn->h_addr_list[0] = apBuffer;
                    
                    in_addr
                        *pAddress = reinterpret_cast<in_addr*>(apBuffer);
                    
                    TInetAddr
                        &address = TInetAddr::Cast(aRecord.iAddr);

                    pAddress->s_addr = ByteOrder::Swap32(address.Address());

                    apBuffer += size;
                    aBufLen  -= size;
                    }
                
                apReturn->h_addr_list[1] = 0;
                
                *appResult = apReturn;
                }
            else
                {
                rc       = -1;
                *apErrno = ERANGE;
                }            
            }
        else
            *apErrno = ERANGE;

        CleanupStack::PopAndDestroy(pName);

        return rc;
        }


    int GetHostByNameL(RHostResolver&   ahResolver,
                       const TDesC8&    aAddress,
                       struct hostent*  apReturn,
                       char*            apBuffer,
                       size_t           aBufLen,
                       struct hostent** appResult,
                       int*             apErrno)
        {
        int
            rc = 0;
        HBufC
            *pAddress = HBufC::NewLC(aAddress.Length());

        pAddress->Des().Copy(aAddress);

        TNameRecord
            record;
        TNameEntry
            entry(record);
        TInt
            error = ahResolver.GetByName(*pAddress, entry);

        switch (error)
            {
            case KErrNone:
                {
                /* FIXME: Currently we only take the first address in account.
                 */
                record = entry();
                
                rc = GetHostByNameL(record,
                                    apReturn,
                                    apBuffer,
                                    aBufLen,
                                    appResult,
                                    apErrno);
                }
                break;
                
            case KErrNotFound:
                {
                *apErrno = HOST_NOT_FOUND;

                rc = -1;
                }
                break;
                
            default:
                User::Leave(error);
            }

        CleanupStack::PopAndDestroy(pAddress);

        return rc;
        }


    int GetHostByNameL(TUint32          aIapId,
                       const TDesC8&    aAddress,
                       struct hostent*  apReturn,
                       char*            apBuffer,
                       size_t           aBufLen,
                       struct hostent** appResult,
                       int*             apErrno)
        {
        int
            rc = 0;
        RSocketServ
            hSs;

        User::LeaveIfError(hSs.Connect());
        CleanupClosePushL(hSs);

        TCommDbConnPref
            pref;

        pref.SetIapId(aIapId);
        pref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);

        RConnection
            hConnection;

        User::LeaveIfError(hConnection.Open(hSs, KAfInet));
        CleanupClosePushL(hConnection);

        User::LeaveIfError(hConnection.Start(pref));
        
        RHostResolver
            hResolver;

        User::LeaveIfError(hResolver.Open(hSs, 
                                          KAfInet, KProtocolInetUdp,
                                          hConnection));
        CleanupClosePushL(hResolver);

        rc = GetHostByNameL(hResolver,
                            aAddress,
                            apReturn,
                            apBuffer,
                            aBufLen,
                            appResult,
                            apErrno);

        CleanupStack::PopAndDestroy(); // hResolver
        CleanupStack::PopAndDestroy(); // hConnection
        CleanupStack::PopAndDestroy(); // hSs

        return rc;
        }


    int GetHostByNameL(CCommsDbTableView& aView,
                       const TDesC8&      aInterface,
                       const TDesC8&      aAddress,
                       struct hostent*    apReturn,
                       char*              apBuffer,
                       size_t             aBufLen,
                       struct hostent**   appResult,
                       int*               apErrno)
        {
        TInt
            rc = 0;
        HBufC
            *pInterface = HBufC::NewLC(aInterface.Length());
        
        pInterface->Des().Copy(aInterface);

        TInt
            error = aView.GotoFirstRecord();
        
        if (error == KErrNone)
            {
            bool
                found = false;
            
            while ((error == KErrNone) && !found)
                {
                TBuf<KCommsDbSvrMaxFieldLength>
                    name;

                aView.ReadTextL(TPtrC(COMMDB_NAME), name);

                if (pInterface->Compare(name) == 0)
                    found = true;
                else
                    error = aView.GotoNextRecord();
                }

            if (found)
                {
                TUint32
                    iapId;
                
                aView.ReadUintL(TPtrC(COMMDB_ID), iapId);

                rc = GetHostByNameL(iapId,
                                    aAddress,
                                    apReturn,
                                    apBuffer,
                                    aBufLen,
                                    appResult,
                                    apErrno);
                }
            else
                error = KErrNotFound;
            }
        
        if (error == KErrNotFound)
            {
            *apErrno = HOST_NOT_FOUND;

            rc = -1;
            }
        else
            User::LeaveIfError(error);

        CleanupStack::PopAndDestroy(pInterface);

        return rc;
        }


    int GetHostByNameL(CCommsDatabase&  aDb,
                       const TDesC8&    aInterface,
                       const TDesC8&    aAddress,
                       struct hostent*  apReturn,
                       char*            apBuffer,
                       size_t           aBufLen,
                       struct hostent** appResult,
                       int*             apErrno)
        {
        int
            rc = 0;

        aDb.ShowHiddenRecords();

        CCommsDbTableView
            *pView = aDb.OpenTableLC(TPtrC(IAP));
        
        rc = GetHostByNameL(*pView,
                            aInterface,
                            aAddress,
                            apReturn,
                            apBuffer,
                            aBufLen,
                            appResult,
                            apErrno);
                            
        CleanupStack::PopAndDestroy(pView);

        return rc;
        }


    int GetHostByNameL(const TDesC8&    aInterface,
                       const TDesC8&    aAddress,
                       struct hostent*  apReturn,
                       char*            apBuffer,
                       size_t           aBufLen,
                       struct hostent** appResult,
                       int*             apErrno)
        {
        int
            rc = 0;
        
        CCommsDatabase
            *pDb = CCommsDatabase::NewL(EDatabaseTypeIAP);
        
        CleanupStack::PushL(pDb);
        
        rc = GetHostByNameL(*pDb,
                            aInterface,
                            aAddress,
                            apReturn,
                            apBuffer,
                            aBufLen,
                            appResult,
                            apErrno);
        
        CleanupStack::PopAndDestroy(pDb);

        return rc;
        }


    int GetHostByName(const TDesC8&    aInterface,
                      const TDesC8&    aAddress,
                      struct hostent*  apReturn,
                      char*            apBuffer,
                      size_t           aBufLen,
                      struct hostent** appResult,
                      int*             apErrno)
        {
        int
            rc = 0;

        *appResult = 0;
        
        TInt
            error;
        
        TRAP(error, rc = GetHostByNameL(aInterface,
                                        aAddress,
                                        apReturn,
                                        apBuffer,
                                        aBufLen,
                                        appResult,
                                        apErrno));

        if (error != KErrNone)
            {
            *apErrno = NO_RECOVERY;
            
            rc = -1;
            }
        
        return rc;
        }
}


namespace
{
                      
    int Copy_h_name(const struct hostent* apResult,
                    struct hostent*       apReturn,
                    char*&                apBuffer,
                    size_t&               aBufLen)
        {
        int
            rc = 0;
        
        if (apResult->h_name)
            rc = Copy_h_name(apResult->h_name, apReturn, apBuffer, aBufLen);
        else
            apReturn->h_name = 0;
        
        return rc;
        }


    int Copy_h_aliases(const struct hostent* apResult,
                       struct hostent*       apReturn,
                       char*&                apBuffer,
                       size_t&               aBufLen)
        {
        int
            rc = 0;

        if (apResult->h_aliases)
            {
            char
                *pEnd = apBuffer + aBufLen;
            int
                n = 0;

            while (apResult->h_aliases[n++])
                ;

            int
                size = n * sizeof(char*);

            if (apBuffer + size < pEnd)
                {
                apReturn->h_aliases = reinterpret_cast<char**>(apBuffer);
                
                apBuffer += size;
                aBufLen  -= size;

                int 
                    i;
                
                for (i = 0; (i < (n - 1)) && (rc == 0); i++)
                    {
                    size = strlen(apResult->h_aliases[i]) + 1;

                    if (apBuffer + size < pEnd)
                        {
                        apReturn->h_aliases[i] = apBuffer;
                        
                        strcpy(apBuffer, apResult->h_aliases[i]);
                        
                        apBuffer += size;
                        aBufLen  -= size;
                        }
                    else
                        rc = -1;
                    }
                
                if (rc == 0)
                    apReturn->h_aliases[i] = 0;
                }
            else
                rc = -1;
            }
        else
            apReturn->h_aliases = 0;

        return rc;
        }
                    
                    
    int Copy_h_addr_list(const struct hostent* apResult,
                         struct hostent*       apReturn,
                         char*&                apBuffer,
                         size_t&               aBufLen)
        {
        int
            rc = 0;

        if (apResult->h_addr_list)
            {
            char
                *pEnd = apBuffer + aBufLen;
            int
                n = 0;

            while (apResult->h_addr_list[n++])
                ;

            int
                size = n * sizeof(char*);

            if (apBuffer + size < pEnd)
                {
                apReturn->h_addr_list = reinterpret_cast<char**>(apBuffer);
                
                apBuffer += size;
                aBufLen  -= size;

                size = apResult->h_length;

                int
                    i;
                
                for (i = 0; (i < (n - 1)) && (rc == 0); i++)
                    {
                    if (apBuffer + size < pEnd)
                        {
                        apReturn->h_addr_list[i] = apBuffer;
                        
                        memcpy(apBuffer, apResult->h_addr_list[i], size);
                        
                        apBuffer += size;
                        aBufLen  -= size;
                        }
                    else
                        rc = -1;
                    }
                
                if (rc == 0)
                    apReturn->h_addr_list[i] = 0;
                }
            else
                rc = -1;
            }
        else
            apReturn->h_addr_list = 0;

        return rc;
        }
                    
                            
    int GetHostByName(const char*      apName,
                      struct hostent*  apReturn,
                      char*            apBuffer,
                      size_t           aBufLen,
                      struct hostent** appResult,
                      int*             apErrno)
        {
        int
            rc = -1;
        TPtrC8
            name(reinterpret_cast<const TUint8*>(apName));
        TInt
            i = name.Locate(':');

        if (i != KErrNotFound)
            {
            TPtrC8
                address   = name.Left(i),
                interface = name.Right(name.Length() - i - 1);
            
            rc = GetHostByName(interface, address, apReturn, 
                               apBuffer, aBufLen, appResult, apErrno);
            }
        else
            {
            *appResult = 0;
            
            struct hostent
                *pHe = gethostbyname(apName); // Threadsafe

            if (pHe)
                {
                memset(apReturn, 0, sizeof(*apReturn));
                
                rc = Copy_h_name(pHe, apReturn, apBuffer, aBufLen);
                
                if (rc == 0)
                    {
                    rc = Copy_h_aliases(pHe, apReturn, apBuffer, aBufLen);

                    if (rc == 0)
                        {
                        apReturn->h_addrtype = pHe->h_addrtype;
                        apReturn->h_length   = pHe->h_length;

                        rc = Copy_h_addr_list(pHe, 
                                              apReturn, apBuffer, aBufLen);
                        }
                    }
                
                if (rc == 0)
                    {
                    *apErrno = 0;

                    *appResult = apReturn;
                    }
                else
                    *apErrno = ERANGE;
                }
            else
                *apErrno = errno;
            }
            
        return rc;
        }
}


extern "C"
{
    int gethostbyname_r(const char*      name,
                        struct hostent*  ret,
                        char*            buf,
                        size_t           buflen,
                        struct hostent** result,
                        int*             h_errnop)
    {
        return GetHostByName(name, ret, buf, buflen, result, h_errnop);
    }


    int gethostbyname_iap_r(const char*      iap,
                            const char*      name,
                            struct hostent*  ret,
                            char*            buf,
                            size_t           buflen,
                            struct hostent** result,
                            int*             h_errnop)
    {
        int rc;
        
        if (iap)
            rc = GetHostByName(TPtrC8(reinterpret_cast<const TUint8*>(iap)),
                               TPtrC8(reinterpret_cast<const TUint8*>(name)),
                               ret, buf, buflen, result, h_errnop);
        else
            rc = gethostbyname_r(name, ret, buf, buflen, result, h_errnop);

        return rc;
    }

}
