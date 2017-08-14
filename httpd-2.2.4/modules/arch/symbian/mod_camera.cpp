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

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"

// APR
#include "apr_strings.h"

// S60
#include <aknquerydialog.h>

// Symbian
#include <ecam.h>
#include <fbs.h>
#include <imageconversion.h>
#include <flogger.h>

#include "s60_mod_utils.h"



using namespace Apache;


/* Camera Module - takes a picture with a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /camera>
 * SetHandler camera
 * </Location>
 *
 * <Location /cameraoptions>
 * SetHandler camera
 * </Location>
 *
 * http://your_server_name/camera
 *
 * /camera   - Returns a new picture.
 *
 */


extern "C"
{

extern module AP_MODULE_DECLARE_DATA s60_camera_module;

}


namespace
{
    const TInt KMaxChunk = 8192;
}


namespace
{
    enum TCameraCommand {
        ECommandUnknown,
        ECommandSnap,
        ECommandOptions
    };

    enum TCameraMode {
        EModeUndefined,
        EModeWebcam,
        EModeInteractive
    };

    typedef TInt TCameraFormat;
    typedef TInt TPictureSize;

    const TCameraMode   KDefaultCameraMode   = EModeInteractive;
    const TCameraFormat KDefaultCameraFormat = 
        CCamera::EFormatFbsBitmapColor64K;
    const TPictureSize  KDefaultPictureSize  = 1;

    const TCameraMode   KUndefinedCameraMode   = EModeUndefined;
    const TCameraFormat KUndefinedCameraFormat = -1;
    const TPictureSize  KUndefinedPictureSize  = -1;
    
    struct TCameraConfig {
        TCameraMode   mode;
        TCameraFormat format;
        TPictureSize  size;
    };
}


namespace
{
    void WriteError(const TDesC8& aMessage, request_rec* r)
        {
        ap_set_content_type(r, "text/html");
        
        ap_rputs(DOCTYPE_HTML_3_2
                 "<html><head>\n"
                 "<title>My Camera</title>\n"
                 "</head>\n"
                 "<body>\n", r);
        
        ap_rputd(aMessage, r);
        
        ap_rputs("</body>", r);
        ap_rputs("</html>", r);
        }


    void SendPicture(HBufC8* apJpeg, request_rec* r)
        {
        ap_set_content_type(r, "image/jpeg");

        const TUint8
            *i   = apJpeg->Ptr(),
            *end = i + apJpeg->Length();
        
        TInt
            nWritten = 0;
            
        do
            {
            TInt
                nWrite = Min(KMaxChunk, (end - i));
            
            nWritten = ap_rwrite(i, nWrite, r);
            
            if (nWritten > 0)
                i += nWritten;
            }
        while ((i != end) && (nWritten > 0));
        }
}


namespace
{
    struct TFormatInfo
        {
        CCamera::TFormat iFormat;
        const char*      ipName;
        } formats[] =
            {
            {
            CCamera::EFormatFbsBitmapColor4K,
            "Color 4K",
            },
            {
            CCamera::EFormatFbsBitmapColor64K,
            "Color 64K",
            },
            {
            CCamera::EFormatFbsBitmapColor16M,
            "Color 16M"
            }
            };
    
        
    const TInt KSizeFormats = sizeof(formats)/sizeof(formats[0]);


    void GetCameraOptionsL(const CCamera&     aCamera,
                           const TCameraInfo& aInfo, 
                           const TFormatInfo& aFormat,
                           request_rec*       r)
        {
        if (aInfo.iImageFormatsSupported & aFormat.iFormat)
            {
            for (TInt i = 0; i < aInfo.iNumImageSizesSupported; i++)
                {
                TSize
                    size;
                
                aCamera.EnumerateCaptureSizes(size, i, aFormat.iFormat);
                
                TBuf8<KMaxIntChars>
                    s;
                
                ap_rputs("  <li> <a href=\"camera?command=snap&", r);
                
                ap_rputs("format=", r);
                s.Num(static_cast<TUint>(aFormat.iFormat));
                ap_rputd(s, r);
                
                ap_rputs("&size=", r);
                s.Num(i);
                ap_rputd(s, r);
                
                ap_rputs("\">", r);
                
                ap_rputs(aFormat.ipName, r);
                ap_rputs(" ", r);
                
                s.Num(size.iWidth);
                ap_rputd(s, r);
                ap_rputs("x", r);
                s.Num(size.iHeight);
                ap_rputd(s, r);
                
                ap_rputs("</a></li>\n", r);
                }
            }
        }
    
    
    void GetCameraOptionsL(const CCamera&     aCamera,
                           const TCameraInfo& aInfo, 
                           request_rec*       r)
        {
        ap_rputs("<h3>Camera Options</h3>\n", r);
        ap_rputs("<ul>\n", r);
        
        for (TInt i = 0; i < KSizeFormats; i++)
            GetCameraOptionsL(aCamera, aInfo, formats[i], r);
        
        ap_rputs("</ul>\n", r);
        }
}


namespace
{
    class CLeicaOptions : public  CBase,
                          private MCameraObserver
        {
    public:
        ~CLeicaOptions()
            {
            if (iPoweredOn)
                ipCamera->PowerOff();
            
            if (iReserved)
                ipCamera->Release();
            
            delete ipCamera;
            delete ipScheduler;
            }
        
        
        static CLeicaOptions* NewL()
            {
            CLeicaOptions
                *pThis = NewLC();
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
    
    
        static CLeicaOptions* NewLC()
            {
            CLeicaOptions
                *pThis = new (ELeave) CLeicaOptions;
            
            CleanupStack::PushL(pThis);
            
            pThis->ConstructL();
            
            return pThis;
            }

        
        void GetL(request_rec* r)
            {
            ir = r;
            
            ReserveCameraL();
            PowerOnCameraL();
            
            TCameraInfo
                info;
            
            ipCamera->CameraInfo(info);
            
            GetCameraOptionsL(*ipCamera, info, ir);
            
            ipCamera->PowerOff();
            iPoweredOn = EFalse;
            
            ipCamera->Release();
            iReserved = EFalse;
            }
            
        
    private:
        CLeicaOptions()
            {
            }
        
        void ConstructL()
            {
            ipScheduler = new (ELeave) CActiveSchedulerWait;
            ipCamera    = CCamera::NewL(*this, 0);
            }


    private:
        void ReserveCameraL()
            {
            iError = KErrNone;
            iStartScheduling = ETrue;
            
            ipCamera->Reserve();
        
            if (iStartScheduling)
                ipScheduler->Start();
            
            if (iError != KErrNone)
                WriteError(_L8("Failed to reserve camera."), ir);
            
            User::LeaveIfError(iError);
            }
        
        
        void PowerOnCameraL()
            {
            iError = KErrNone;
            iStartScheduling = ETrue;
            
            ipCamera->PowerOn();
            
            if (iStartScheduling)
                ipScheduler->Start();
            
            if (iError != KErrNone)
                WriteError(_L8("Failed to power on camera."), ir);
            
            User::LeaveIfError(iError);
            }

        
    private:
        void ReserveComplete(TInt aError)
            {
            iError = aError;
            
            if (ipScheduler->IsStarted())
                ipScheduler->AsyncStop();
            else
                iStartScheduling = EFalse;
            
            if (iError == KErrNone)
                iReserved = ETrue;
            }
        
        
        void PowerOnComplete(TInt aError)
            {
            iError = aError;
            
            if (ipScheduler->IsStarted())
                ipScheduler->AsyncStop();
            else
                iStartScheduling = EFalse;
            
            if (iError == KErrNone)
                iPoweredOn = ETrue;
            }
        
        
        void ViewFinderFrameReady(CFbsBitmap&)
            {
            }


        void ImageReady(CFbsBitmap*, HBufC8*, TInt)
            {
            }


        void FrameBufferReady(MFrameBuffer*, TInt)
            {
            }

    private:
        request_rec*          ir;
        CActiveSchedulerWait* ipScheduler;
        CCamera*              ipCamera;
        TInt                  iError;
        TBool                 iStartScheduling;
        TBool                 iReserved;
        TBool                 iPoweredOn;
        };
}


namespace
{
    class CLeica : public  CBase,
                   private MCameraObserver
        {
    public:
        class MCallback
            {
        public:
            virtual void ReserveCameraError  (CLeica*, TInt rc) = 0;
            virtual void ReserveCameraSuccess(CLeica*) {}

            virtual void PowerOnError        (CLeica*, TInt rc) = 0;
            virtual void PowerOnSuccess      (CLeica*) {}

            virtual void CaptureImageError   (CLeica*, TInt rc) = 0;
            virtual void CaptureImageSuccess (CLeica*, 
                                              CFbsBitmap* apBitmap) = 0;
            };


        ~CLeica()
            {
            delete ipCamera;
            }
        
        
        static CLeica* NewL()
            {
            CLeica
                *pThis = NewLC();
            
            CleanupStack::Pop(pThis);
            
            return pThis;
            }
        
    
        static CLeica* NewLC()
            {
            CLeica
                *pThis = new (ELeave) CLeica;
            
            CleanupStack::PushL(pThis);
            
            pThis->ConstructL();
            
            return pThis;
            }
    
    
        void TakePictureL(CCamera::TFormat aFormat, 
                          TInt             aSizeIndex, 
                          MCallback*       apCallback)
            {
            if (ipCallback)
                User::Leave(KErrInUse);
            
            iFormat    = aFormat;
            iSizeIndex = aSizeIndex;
            ipCallback = apCallback;
            
            ipCamera->Reserve();
            }


    private:
        CLeica()
            {
            }
        

        void ConstructL()
            {
            ipCamera = CCamera::NewL(*this, 0);
            }

        
        void Reset()
            {
            iFormat    = static_cast<CCamera::TFormat>(0);
            iSizeIndex = 0;
            ipCallback = 0;
            }
        

    private:
        void ReserveComplete(TInt aError)
            {
            if (aError == KErrNone)
                {
                ipCallback->ReserveCameraSuccess(this);
                
                ipCamera->PowerOn();
                }
            else
                {
                ipCallback->ReserveCameraError(this, aError);
                
                Reset();
                }
            }
        
        
        void PowerOnComplete(TInt aError)
            {
            if (aError == KErrNone)
                {
                ipCallback->PowerOnSuccess(this);
                
#if !defined(__WINS__)
                TRAP(aError, ipCamera->PrepareImageCaptureL(iFormat, 
                                                            iSizeIndex));
                
                if (aError == KErrNone)
                    ipCamera->CaptureImage();
                else
                    ipCallback->CaptureImageError(this, aError);
#else
                CFbsBitmap
                    *pBitmap = new (ELeave) CFbsBitmap;
                TSize
                    size(1280, 960);
                
                TInt
                    rc = pBitmap->Create(size, EColor16M);
                
                if (rc == KErrNone)
                    {
                    TUint32
                        *pDataAddress = pBitmap->DataAddress();
                    TUint8
                        *i   = reinterpret_cast<TUint8*>(pDataAddress),
                        *end = i + 1280 * 960 * 3; // 3 = 24bps / 8 bits/byte
                    
                    while (i != end) 
                    {
                    *i = reinterpret_cast<TUint32>(i);
                    
                    ++i;
                    }
                
                ipCallback->CaptureImageSuccess(this, pBitmap);
                }
            else
                ipCallback->CaptureImageError(this, rc);

            ipCamera->PowerOff();
            ipCamera->Release();

            Reset();
#endif
            }
        else
            {
            ipCamera->Release();
            
            ipCallback->PowerOnError(this, aError);

            Reset();
            }
        }
    
    
    void ViewFinderFrameReady(CFbsBitmap& aFrame)
        {
        }
    
    
    void ImageReady(CFbsBitmap* apBitmap, HBufC8* apData, TInt aError)
        {
        ipCamera->PowerOff();
        ipCamera->Release();
        
        if (aError == KErrNone)
            ipCallback->CaptureImageSuccess(this, apBitmap);
        else
            ipCallback->CaptureImageError(this, aError);

        Reset();
        }

    
    void FrameBufferReady(MFrameBuffer* apFrameBuffer, TInt aError)
        {
        }

    private:
        CCamera*         ipCamera;
        CCamera::TFormat iFormat;
        TInt             iSizeIndex;
        MCallback*       ipCallback;
        };
}


namespace
{
    class CShowAndTell : public  CBase,
                         private CLeica::MCallback,
                         private CMessage::MCallback,
                         private CImageConverter::MCallback
        {
    public:
        ~CShowAndTell()
            {
            delete ipCamera;
            delete ipMessage;
            delete ipConverter;
            delete ipScheduler;
            }


        static CShowAndTell* NewL()
            {
            CShowAndTell
                *pThis = NewLC();
            
            CleanupStack::Pop(pThis);

            return pThis;
            }
        

        static CShowAndTell* NewLC()
            {
            CShowAndTell
                *pThis = new (ELeave) CShowAndTell;
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();

            return pThis;
            }

        
        void SnapL(CCamera::TFormat aFormat,
                   TInt             aSizeIndex,
                   TBool            aInteractive, 
                   request_rec*     r)
            {
            if (ir)
                User::Leave(KErrInUse);
            
            ir = r;

            if (aInteractive)
                ipMessage->ShowL(_L("Webcam"),
                                 _L("Hold Steady..."),
                                 R_AVKON_SOFTKEYS_EMPTY,
                                 CMessage::KInfinite,
                                 CAknQueryDialog::ENoTone,
                                 this);
            
            ipCamera->TakePictureL(aFormat, aSizeIndex, this);
            
            ipScheduler->Start();

            ir = 0;
            }


    private:
        CShowAndTell()
            {
            }

        void ConstructL()
            {
            ipScheduler = new (ELeave) CActiveSchedulerWait;
            ipMessage = CMessage::NewL();
            ipCamera = CLeica::NewL();
            ipConverter = CImageConverter::NewL();
            }

        void Stop()
            {
            ipScheduler->AsyncStop();
            }

    private:
        void AcknowledgedL(CMessage*, TInt)
            {
            Stop();
            }

    private:
        void ReserveCameraError(CLeica*, TInt rc)
            {
            WriteError(_L8("Failed to reserve camera."), ir);

            if (ipMessage->IsActive())
                ipMessage->Cancel();
            
            Stop();
            }

        
        void PowerOnError(CLeica*, TInt rc)
            {
            WriteError(_L8("Failed to power on camera."), ir);
            
            if (ipMessage->IsActive())
                ipMessage->Cancel();

            Stop();
            }
        
        void CaptureImageError(CLeica*, TInt rc)
            {
            WriteError(_L8("Failed to capture image."), ir);
            
            if (ipMessage->IsActive())
                ipMessage->Cancel();

            Stop();
            }

        void CaptureImageSuccess(CLeica*, CFbsBitmap* apBitmap)
            {
            ipBitmap = apBitmap;
            
            if (ipMessage->IsActive())
                ipMessage->Cancel();

            ipConverter->ConvertL(*ipBitmap, _L8("image/jpeg"), this);
            }

    private:
        void ConvertSuccess(CImageConverter*, HBufC8* apImage)
            {
            delete ipBitmap;
            ipBitmap = 0;

            SendPicture(apImage, ir);
            
            delete apImage;
            
            Stop();
            }


        void ConvertError(CImageConverter*, TInt aError)
            {
            WriteError(_L8("Failed to convert image."), ir);

            delete ipBitmap;
            
            Stop();
            }
        

    private:
        CActiveSchedulerWait* ipScheduler;
        CMessage*             ipMessage;
        CLeica*               ipCamera;
        TInt                  inActive;
        CFbsBitmap*           ipBitmap;
        CImageConverter*      ipConverter;
        request_rec*          ir;
        };
}


namespace
{
    int GeneratePictureL(request_rec*     r, 
                         CCamera::TFormat aFormat, 
                         TInt             aSizeIndex)
        {
        CShowAndTell
            *pShow = CShowAndTell::NewLC();
        
        pShow->SnapL(aFormat, aSizeIndex, EFalse, r);
        
        CleanupStack::PopAndDestroy(pShow);

        return OK;
        }


    namespace
        {
            _LIT(KUserHead,     "Take a picture for ");
            _LIT(KUserSomebody, "somebody");
            _LIT(KUserTail,     "?");
            
            const TInt KTimeout = 10;


            HBufC* GetPromptLC(request_rec* r)
                {
                TInt
                    size   = KUserHead.BufferSize + KUserTail.BufferSize,
                    length = 0;
                
                if (r->user)
                    {
                    length = strlen(r->user);
                    
                    size += length;
                    }
                else
                    size += KUserSomebody.BufferSize;
                
                HBufC
                    *pPrompt = HBufC::NewLC(size);
                
                pPrompt->Des().Append(KUserHead);
                
                if (r->user)
                    {
                    HBufC
                        *pUser = HBufC::NewLC(length);
                    TPtrC8
                        user(reinterpret_cast<TUint8*>(r->user), length);
                    
                    pUser->Des().Copy(user);
                    
                    pPrompt->Des().Append(*pUser);
                    
                    CleanupStack::PopAndDestroy(pUser);
                    }
                else
                    pPrompt->Des().Append(KUserSomebody);
                
                pPrompt->Des().Append(KUserTail);
                
                return pPrompt;
                }
        }
    

    int TakePictureL(request_rec*     r, 
                     CCamera::TFormat aFormat, 
                     TInt             aSizeIndex)
        {
        int
            rc = OK;
        CConfirmer
            *pConfirmer = CConfirmer::NewLC();
        HBufC
            *pPrompt = GetPromptLC(r);
        TInt
            result = pConfirmer->ConfirmL(*pPrompt, 
                                          R_AVKON_SOFTKEYS_OK_CANCEL,
                                          TTimeIntervalSeconds(KTimeout));

        switch (result)
            {
            case EAknSoftkeyOk:
                {
                CShowAndTell
                    *pShow = CShowAndTell::NewLC();
                
                pShow->SnapL(aFormat, aSizeIndex, ETrue, r);
                
                CleanupStack::PopAndDestroy(pShow);
                }
                break;

            case CConfirmation::KTimedOut:
                // FIXME: Deal with timeouts separately.
            case EAknSoftkeyCancel:
                rc = HTTP_FORBIDDEN;
            }

        CleanupStack::PopAndDestroy(pPrompt);
        CleanupStack::PopAndDestroy(pConfirmer);
        
        return rc;
        }
}


namespace
{
    typedef int (*Handler)(request_rec*, CCamera::TFormat, TInt);

    
    void GetMode(const TCameraConfig& config,
                 TCameraMode*         pMode)
        {
        if (config.mode == KUndefinedCameraMode)
            *pMode = KDefaultCameraMode;
        else
            *pMode = config.mode;
        }


    int GetFormat(const TCameraConfig& config,
                  QueryArgument*       pArguments, 
                  int                  nArguments,
                  TInt*                pFormat)
        {
        int
            rc = OK;
        
        if (config.format == KUndefinedCameraFormat)
            {
            const char
                *pValue;

            if (GetValue(pArguments, nArguments, "format", &pValue))
                {
                TLex8
                    lexer(reinterpret_cast<const TUint8*>(pValue));
                
                if (lexer.Val(*pFormat) != KErrNone)
                    rc = HTTP_BAD_REQUEST;
                }
            else
                *pFormat = KDefaultCameraFormat;
            }
        else
            *pFormat = config.format;

        return rc;
        }
    

    int GetSize(const TCameraConfig& config,
                QueryArgument*       pArguments, 
                int                  nArguments,
                TInt*                pSize)
        {
        int
            rc = OK;
        
        if (config.size == KUndefinedPictureSize)
            {
            const char
                *pValue;

            if (GetValue(pArguments, nArguments, "size", &pValue))
                {
                TLex8
                    lexer(reinterpret_cast<const TUint8*>(pValue));
                
                if (lexer.Val(*pSize) != KErrNone)
                    rc = HTTP_BAD_REQUEST;
                }
            else
                *pSize = KDefaultPictureSize;
            }
        else
            *pSize = config.size;

        return rc;
        }
    

    int PictureHandlerL(request_rec*     r,
                        TCameraMode      mode,
                        CCamera::TFormat format,
                        TInt             size)
        {
        int
            rc = OK;
        
        if (mode == EModeWebcam)
            rc = GeneratePictureL(r, format, size);
        else
            rc = TakePictureL(r, format, size);
        
        return rc;
        }

    
    int PictureHandlerL(request_rec*         r,
                        const TCameraConfig& config,
                        QueryArgument*       pArguments, 
                        int                  nArguments)
        {
        int
            rc = OK;

        TCameraMode
            mode;
        
        GetMode(config, &mode);
        
        TInt
            format;

        rc = GetFormat(config, pArguments, nArguments, &format);

        if (rc == OK)
            {
            TInt
                size;

            rc = GetSize(config, pArguments, nArguments, &size);

            if (rc == OK)
                rc = PictureHandlerL(r, 
                                     mode,
                                     static_cast<CCamera::TFormat>(format),
                                     size);
            }
        
        return rc;
        }


    int OptionsHandlerL(request_rec* r)
        {
        CLeicaOptions
            *pOptions = CLeicaOptions::NewLC();

        ap_set_content_type(r, "text/html");

        ap_rputs(DOCTYPE_HTML_3_2
                 "<html><head>\n"
                 "<title>Camera Options</title>\n"
                 "</head>\n"
                 "<body>\n", r);
        
        pOptions->GetL(r);

        ap_rputs("</body>", r);
        ap_rputs("</html>", r);

        CleanupStack::PopAndDestroy(pOptions);

        return OK;
        }
}


namespace
{
    int CameraHandlerL(request_rec* r, const TCameraConfig& config)
        {
        QueryArgument
            *pArguments;
        int
            nArguments;
        int
            rc = ParseQueryString(r, &pArguments, &nArguments);
        
        if (rc == OK)
            {
            rc = HTTP_BAD_REQUEST;

            TCameraCommand
                command = ECommandSnap;
            
            const char
                *pCommand;
            
            if (GetValue(pArguments, nArguments, "command", &pCommand))
                {
                command = ECommandUnknown;
                
                if (strcmp(pCommand, "snap") == 0)
                    command = ECommandSnap;
                else if (strcmp(pCommand, "options") == 0)
                    command = ECommandOptions;
                }

            switch (command)
                {
                case ECommandUnknown:
                    break;
                    
                case ECommandSnap:
                    rc = PictureHandlerL(r, config, pArguments, nArguments);
                    break;

                case ECommandOptions:
                    rc = OptionsHandlerL(r);
                    break;
                }
            }

        return rc;
        }
}


namespace
{
    int RequestHandlerL(request_rec* r)
        {
        int
            rc = OK;
        TInt
            error = RFbsSession::Connect();
        
        if (error == KErrNone)
            {
            void
                *pData = ap_get_module_config(r->per_dir_config,
                                              &s60_camera_module);
            TCameraConfig
                *pConfig = static_cast<TCameraConfig*>(pData);

            TRAP(error, rc = CameraHandlerL(r, *pConfig));
            
            switch (error)
                {
                case KErrNone:
                    break;
                    
                case KErrArgument:
                    rc = HTTP_BAD_REQUEST;
                    break;
                    
                default:
                    rc = HTTP_INTERNAL_SERVER_ERROR;
                }
            
            RFbsSession::Disconnect();
            }
        else
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        return rc;
        }
    
    
    int RequestHandler(request_rec* r)
        {
        __UHEAP_MARK;
        
        int 
            rc = HTTP_INTERNAL_SERVER_ERROR;
        
        TRAPD(error, rc = RequestHandlerL(r));
        
        __UHEAP_MARKEND;
        
        return rc;
        }
}


namespace
{
    int camera_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "camera") == 0) {
            rc = RequestHandler(r);
        }
        
        return rc;
    }


    void* create_camera_config(apr_pool_t* p, char* dummy) {
        TCameraConfig
            *pConfig = (TCameraConfig*) apr_pcalloc(p, sizeof(*pConfig));

        if (pConfig) {
            pConfig->mode   = KUndefinedCameraMode;
            pConfig->format = KUndefinedCameraFormat;
            pConfig->size   = KUndefinedPictureSize;
        }

        return pConfig;
    }


    void *merge_camera_configs(apr_pool_t *p, void *basev, void *addv) {
        TCameraConfig
            *pNew,
            *pBase = static_cast<TCameraConfig*>(basev),
            *pAdd  = static_cast<TCameraConfig*>(addv);

        pNew = (TCameraConfig*) apr_pcalloc(p, sizeof(*pNew));

        if (pNew) {
            if (pAdd->mode != KUndefinedCameraMode)
                pNew->mode = pAdd->mode;
            else
                pNew->mode = pBase->mode;
            
            if (pAdd->format != KUndefinedCameraFormat)
                pNew->format = pAdd->format;
            else
                pNew->format = pBase->format;
            
            if (pAdd->size != KUndefinedPictureSize)
                pNew->size = pAdd->size;
            else
                pNew->size = pBase->size;
        }

        return pNew;
    }

    
    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(camera_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }

    
    const char* add_options(cmd_parms* cmd, void* d, const char* optstr) {
        TCameraConfig
            *pConfig = static_cast<TCameraConfig*>(d);

        while (optstr[0]) {
            char
                *pOption = ap_getword_conf(cmd->pool, &optstr);

            if (strncasecmp(pOption, "Mode=", 5) == 0) {
                char
                    *pValue = &pOption[5];
                
                if (strcasecmp(pValue, "interactive") == 0)
                    pConfig->mode = EModeInteractive;
                else if (strcasecmp(pValue, "webcam") == 0)
                    pConfig->mode = EModeWebcam;
                else
                    return "value for Mode is either interactive or webcam";
                    
            }
            else if (strncasecmp(pOption, "Format=", 7) == 0) {
                char
                    *pValue = &pOption[7];

                pConfig->format = atoi(pValue);
            }
            else if (strncasecmp(pOption, "Size=", 5) == 0) {
                char
                    *pValue = &pOption[5];

                pConfig->size = atoi(pValue);
            }
            else 
                return "Invalid CameraOptions option";
        }
        
        return 0;
    }


    const command_rec camera_cmds[] = {
        AP_INIT_ITERATE("CameraOptions", add_options, NULL, 
                        OR_LIMIT | OR_OPTIONS,
                        "one or more camera options"),
        {0}
    };
}


extern "C"
{

module AP_MODULE_DECLARE_DATA s60_camera_module =
{
    STANDARD20_MODULE_STUFF,
    create_camera_config,       /* dir config creater */
    merge_camera_configs,       /* dir merger --- default is to override */
    NULL,                       /* server config */
    NULL,                       /* merge server config */
    camera_cmds,                /* command table */
    register_hooks              /* register_hooks */
};

}
