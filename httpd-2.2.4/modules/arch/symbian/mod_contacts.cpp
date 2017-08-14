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

// Symbian
#include <cntdb.h>
#include <cntitem.h>
#include <cntfldst.h>

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Contacts Module - displays the contacts on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /contacts>
 * SetHandler contacts
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/contacts
 *
 * /contacts   - Returns all contacts as a clickable list.
 * /contacts?n - Returns contact n (integer).
 *
 */


namespace
{
    // Predefined items than can be found in a contact.
    const TInt32 uids[] =
    {
    KUidContactFieldAddressValue,
    KUidContactFieldPostOfficeValue,
    KUidContactFieldExtendedAddressValue,
    KUidContactFieldLocalityValue,
    KUidContactFieldRegionValue,
    KUidContactFieldPostCodeValue,
    KUidContactFieldCountryValue,
    KUidContactFieldCompanyNameValue,
    KUidContactFieldPhoneNumberValue,
    KUidContactFieldGivenNameValue,
    KUidContactFieldFamilyNameValue,
    KUidContactFieldAdditionalNameValue,
    KUidContactFieldSuffixNameValue,
    KUidContactFieldPrefixNameValue,
    KUidContactFieldHiddenValue,
    KUidContactFieldDefinedTextValue,
    KUidContactFieldEMailValue,
    KUidContactFieldMsgValue,
    KUidContactFieldSmsValue,
    KUidContactFieldFaxValue,
    KUidContactFieldNoteValue,
    KUidContactFieldBirthdayValue,
    KUidContactFieldUrlValue,
    KUidContactFieldTemplateLabelValue,
    KUidContactFieldPictureValue,
    KUidContactFieldDTMFValue,
    KUidContactFieldRingToneValue,
    KUidContactFieldJobTitleValue,
    KUidContactFieldIMAddressValue
    };
    
    const TInt KUidsCount = sizeof(uids)/sizeof(uids[0]);

    const TInt KMaxIntChars = 10; // strlen("4294967296")
}


//
// Here we generate information about 1 contact!
//
namespace
{
    void GenerateNotFoundL(request_rec* r)
        {
        ap_rputs("Unknown contact!", r);
        }


    void GenerateContactFieldL(const CContactItemField& aField, request_rec* r)
        {
        TPtrC
            label = aField.Label();

        if (label.Length())
            {
            switch (aField.StorageType())
                {
                case KStorageTypeText:
                    {
                    TPtrC
                        text = aField.TextStorage()->Text();
                    
                    if (text.Length())
                        {
                        ap_rputs("  <li><b>", r);
                        ap_rputdL(label, r);
                        ap_rputs(": </b>", r);
                        ap_rputdL(text, r);
                        ap_rputs("</li>", r);
                        }
                    }
                    break;

                case KStorageTypeStore:
                case KStorageTypeContactItemId:
                case KStorageTypeDateTime:
                    // FIXME: Deal with these properly.
                    break;
                }
            }
        }


    void GenerateContactL(CContactItem& aContact, request_rec* r)
        {
        CContactItemFieldSet
            &fields = aContact.CardFields();
        
        ap_rputs("<ul>\n", r);

        for (TInt i = 0; i < KUidsCount; ++i)
            {
            TInt
                index = fields.Find(TUid::Uid(uids[i]));
            
            if (index != KErrNotFound)
                {
                const CContactItemField
                    &field = fields[index];
                
                GenerateContactFieldL(field, r);
                }
            }

        ap_rputs("</ul>\n", r);
        }


    void GenerateContactL(CContactDatabase& aDb,
                          TContactItemId    aId, 
                          request_rec*      r)
        {
        CContactItem
            *pContact = aDb.ReadContactLC(aId);

        GenerateContactL(*pContact, r);
        
        CleanupStack::PopAndDestroy(pContact);
        }


    int GenerateContactL(request_rec* r) 
        {
        CContactDatabase
            *pDb = CContactDatabase::OpenL();

        CleanupStack::PushL(pDb);

        TLex8
            lexer(reinterpret_cast<const TUint8*>(r->args));
        TInt
            id;
        TInt
            rc = lexer.Val(id);
        
        if (rc == KErrNone)
            {
            TRAPD(rc, GenerateContactL(*pDb, id, r));
            
            if (rc != KErrNone)
                GenerateNotFoundL(r);
            }
        else
            GenerateNotFoundL(r);

        CleanupStack::PopAndDestroy(pDb);

        return OK;
    }


}


//
// Here we generate information about all contacts!
//
namespace
{
    void GenerateContactRowL(CContactItem* apContact, request_rec* r) 
        {
        CContactItemFieldSet
            &fields = apContact->CardFields();
        TInt
            iFamily = fields.Find(KUidContactFieldFamilyName),
            iName   = fields.Find(KUidContactFieldGivenName);
        
        if ((iFamily != KErrNotFound) || (iName != KErrNotFound))
            {
            ap_rputs("  <li> <a href=\"contacts?", r);
            
            TInt
                id = apContact->Id();
            TBuf8<KMaxIntChars>
                idString;
            
            idString.Num(id);
            
            ap_rwrite(idString.Ptr(), idString.Length(), r);
            
            ap_rputs("\">", r);
                
            if (iFamily != KErrNotFound)
                {
                TPtrC
                    family = fields[iFamily].TextStorage()->Text();

                if (family.Length() != 0)
                    {
                    ap_rputdL(family, r);

                    if (iName != KErrNotFound)
                        ap_rputs(" ", r);
                    }
                }
            
            if (iName != KErrNotFound)
                {
                TPtrC
                    name = fields[iName].TextStorage()->Text();
                
                if (name.Length() != 0)
                    ap_rputdL(name, r);
                }
            
            ap_rputs(static_cast<const char*>("</li>\n"), r);
            }
    }
        

    int GenerateContactListL(request_rec* r) 
        {
        CContactDatabase
            *pDb = CContactDatabase::OpenL();

        CleanupStack::PushL(pDb);
        
        TContactIter
            i(*pDb);
        
        TContactItemId
            id;
        
        ap_rputs("<ul>\n", r);
        
        while ((id = i.NextL()) != KNullContactId) 
            {
            CContactItem
                *pContact = pDb->ReadContactL(id);
            
            CleanupStack::PushL(pContact);

            GenerateContactRowL(pContact, r);

            pDb->CloseContactL(pContact->Id());
            
            CleanupStack::PopAndDestroy(pContact);
            }

        ap_rputs("</ul>\n", r);

        CleanupStack::PopAndDestroy(pDb);

        return OK;
    }
}


namespace
{
    int GenerateContentL(request_rec* r)
        {
        int
            rc;
        
        if (r->args)
            rc = GenerateContactL(r);
        else
            rc = GenerateContactListL(r);
        
        return rc;
        }
    
    
    int ContactsHandler(request_rec* r)
        {
        __UHEAP_MARK;
        
        int
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        TRAPD(error, rc = GenerateContentL(r));
            
        __UHEAP_MARKEND;
        
        return rc;
        }
}


namespace
{
    int contacts_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "contacts") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html><head>\n"
                     "<title>My Contacts</title>\n"
                     "</head>\n"
                     "<body>\n", r);
            
            rc = ContactsHandler(r);
        
            ap_rputs("</body>", r);
            ap_rputs("</html>", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(contacts_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{

module AP_MODULE_DECLARE_DATA symbian_contacts_module =
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
