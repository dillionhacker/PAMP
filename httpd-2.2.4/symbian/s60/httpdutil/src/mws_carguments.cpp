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

#include <mws/carguments.h>


namespace Mws
{
    EXPORT_C CArguments1::CArguments1()
        {
        }


    EXPORT_C CArguments1::~CArguments1()
        {
        delete ipValue;
        }


    EXPORT_C void CArguments1::ConstructL()
        {
        ipValue = KNullDesC().AllocL();
        }
    

    EXPORT_C CArguments1* CArguments1::NewL()
        {
        CArguments1
            *pThis = NewLC();

        CleanupStack::Pop(pThis);

        return pThis;
        }


    EXPORT_C CArguments1* CArguments1::NewLC()
        {
        CArguments1
            *pThis = new (ELeave) CArguments1;

        CleanupStack::PushL(pThis);
        pThis->ConstructL();

        return pThis;
        }

    
    EXPORT_C const TDesC& CArguments1::Get() const
        {
        return *ipValue;
        }

    
    EXPORT_C void CArguments1::SetL(const TDesC& aValue)
        {
        HBufC
            *pPrev = ipValue;

        ipValue = aValue.AllocL();

        delete pPrev;
        }


    EXPORT_C void CArguments1::ReadDataL(RReadStream& ahStream)
        {
        TInt
            length = ahStream.ReadInt32L();
        
        if (length)
            {
            HBufC
                *pOld = ipValue,
                *pNew = HBufC::NewMaxL(length);
            TPtr
                des = pNew->Des();

            ahStream.ReadL(des, length);
            
            ipValue = pNew;
            
            delete pOld;
            }
        else
            ipValue->Des().SetLength(0);
        }


    EXPORT_C void CArguments1::WriteDataL(RWriteStream& ahStream) const
        {
        TInt
            length = ipValue->Length();

        ahStream.WriteInt32L(length);
        
        if (length)
            ahStream.WriteL(*ipValue);
        }

        
    EXPORT_C void CArguments1::ExternalizeL(RWriteStream& ahStream) const
        {
        ahStream.WriteInt32L(KVersion);

        WriteDataL(ahStream);
        }


    EXPORT_C void CArguments1::ExternalizeL(const TDesC& aPath) const
        {
        TInt
            rc;
        RFs
            hFs;
        
        rc = hFs.Connect();
        
        User::LeaveIfError(rc);
        CleanupClosePushL(hFs);
        
        RFile
            hFile;

        rc = hFile.Open(hFs, aPath, EFileWrite);
        
        if (rc == KErrNotFound)
            rc = hFile.Create(hFs, aPath, EFileWrite);
        
        User::LeaveIfError(rc);
        
        CleanupClosePushL(hFile);
        
        RFileWriteStream
            hStream;
        
        hStream.Attach(hFile);
        hStream.PushL();
        
        ExternalizeL(hStream);
        
        hStream.CommitL();
        
        CleanupStack::PopAndDestroy(3); // hStream, hFile, hFs
        }
    

    EXPORT_C void CArguments1::InternalizeL(const TDesC& aPath)
        {
        TInt
            rc;
        RFs
            hFs;
        
        rc = hFs.Connect();
        
        User::LeaveIfError(rc);
        CleanupClosePushL(hFs);
        
        RFile
            hFile;
            
        rc = hFile.Open(hFs, aPath, EFileRead);
        
        switch (rc)
            {
            case KErrNone:
            case KErrNotFound:
                break;

            default:
                User::Leave(rc);
            }
            
        if (rc == KErrNone)
            {
            CleanupClosePushL(hFile);
                
            RFileReadStream
                hStream;
            
            hStream.Attach(hFile);
            hStream.PushL();
            
            InternalizeL(hStream);
            
            CleanupStack::PopAndDestroy(2); // hStream, hFile
            }
        
        CleanupStack::PopAndDestroy(1); // hFs
        }
        

    EXPORT_C void CArguments1::InternalizeL(RReadStream& ahStream)
        {
        TInt32
            version = ahStream.ReadInt32L();

        if (version != KVersion)
            User::Leave(KErrCorrupt);

        ReadDataL(ahStream);
        }


    EXPORT_C void CArguments1::CopyL(const CArguments1& aRhs)
        {
        SetL(aRhs.Get());
        }
}
