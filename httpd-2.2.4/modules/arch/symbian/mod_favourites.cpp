/* Copyright 2006 Nokia Corporation
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

#include "symbian_mod_utils.h"
#if defined(EKA2)
#include <favouritessession.h>
#endif
#include <favouritesdb.h>
#include <favouritesitemlist.h>

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Messages Module - displays the messages on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /favourites>
 * SetHandler favourites
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/favourites
 *
 * /favourites - Returns the favourites of your browser.
 *
 */


namespace
{
#if defined(EKA2)

_LIT(KFavouritesDbName, "browserbookmarks");


class CFavourites : public CBase
    {
public:
    ~CFavourites()
        {
        mhSession.Close();
        }

    static CFavourites* NewL()
        {
        CFavourites
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    static CFavourites* NewLC()
        {
        CFavourites
            *pThis = new (ELeave) CFavourites;

        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }

    void OpenL()
        {
        User::LeaveIfError(mhDb.Open(mhSession, KFavouritesDbName));
        }

    void Close()
        {
        mhDb.Close();
        }

    void GetAllL(
        CFavouritesItemList&   aItemList,
        TInt                   aParentFolder = KFavouritesNullUid,
        CFavouritesItem::TType aTypeFilter = CFavouritesItem::ENone,
        const TDesC*           aNameFilter = 0,
        TInt32                 aContextIdFilter = KFavouritesNullContextId)
        {
        User::LeaveIfError(mhDb.GetAll(aItemList,
                                       aParentFolder,
                                       aTypeFilter,
                                       aNameFilter,
                                       aContextIdFilter));
        }
                 
private:
    void ConstructL()
        {
        User::LeaveIfError(mhSession.Connect());
        }

private:
    RFavouritesSession mhSession;
    RFavouritesDb      mhDb;
    };

#else

_LIT(KBookmarkDbDir,    "C:\\System\\Data\\");
_LIT(KBookmarkDbName,   "Bookmarks1.db");


class CFavourites : public CFavouritesDb
    {
public:
    static CFavourites* NewL()
        {
        CFavourites
            *pThis = NewLC();
        
        CleanupStack::Pop(pThis);
        
        return pThis;
        }
    
    static CFavourites* NewLC()
        {
        CFavourites
            *pThis = new (ELeave) CFavourites;

        CleanupStack::PushL(pThis);
        pThis->ConstructL();
        
        return pThis;
        }
    
    TUid PinbLinkUid() const
        {
        return TUid();
        }
    
    TPinbDocumentIconId PinbLinkIcon() const
        {
        return EPinbBookMarkIcon;
        }
    
private:
    void ConstructL()
        {
        CFavouritesDb::ConstructL(KBookmarkDbDir, KBookmarkDbName);
        }
    };
#endif

}


namespace
{
    using namespace Apache;


    void FavouritesHandlerL(CFavourites&         aDb,
                            CFavouritesItemList& aItems, 
                            request_rec*         r)
        {
        if (aItems.Count() != 0)
            {
            ap_rputs("<ul>\n", r);
            
            for (TInt i = 0; i < aItems.Count(); i++)
                {
                CFavouritesItem
                    &item = *aItems.At(i);
                
                ap_rputs("<li>", r);

                if (item.IsFolder())
                    {
                    ap_rputdL(item.Name(), r);
                    
                    CFavouritesItemList
                        *pItems = new (ELeave) CFavouritesItemList;
                    
                    CleanupStack::PushL(pItems);
                    
                    aDb.GetAllL(*pItems, item.Uid());
                    
                    FavouritesHandlerL(aDb, *pItems, r);
                    
                    CleanupStack::PopAndDestroy(pItems);
                    }
                else
                    {
                    ap_rputs("<a href=\"", r);
                    ap_rputdL(item.Url(), r);
                    ap_rputs("\">", r);
                    ap_rputdL(item.Name(), r);
                    ap_rputs("</a>", r);
                    }

                
                ap_rputs("</li>\n", r);
                }

            ap_rputs("</ul>\n", r);
            }
        }


    int FavouritesHandlerL(request_rec* r)
        {
        CFavourites
            *pDb = CFavourites::NewLC();
        CFavouritesItemList
            *pItems = new (ELeave) CFavouritesItemList;
        
        CleanupStack::PushL(pItems);

        pDb->OpenL();
        
        CleanupClosePushL(*pDb);
        
        pDb->GetAllL(*pItems, KFavouritesRootUid);

        FavouritesHandlerL(*pDb, *pItems, r);
        
        CleanupStack::PopAndDestroy(); // pDb->Close();
        CleanupStack::PopAndDestroy(pItems);
        CleanupStack::PopAndDestroy(pDb);
        
        return OK;
        }


    int FavouritesHandler(request_rec* r)
        {
        __UHEAP_MARK;
        
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        TRAPD(error, rc = FavouritesHandlerL(r));
        
        __UHEAP_MARKEND;

        return rc;
        }
}


namespace
{
    int favourites_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "favourites") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html>\n"
                     "  <head>\n"
                     "    <title>Favourites</title>\n"
                     "  </head>\n"
                     "  <body>\n", r);
            
            rc = FavouritesHandler(r);
            
            ap_rputs("  </body>\n", r);
            ap_rputs("</html>\n", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t*) {
        ap_hook_handler(favourites_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA s60_favourites_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                       /* dir config creater */
    NULL,                       /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server config */
    NULL,                       /* command table */
    register_hooks              /* register_hooks */
};

}
