/* Copyright 2007 Nokia Corporation
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

#include <e32base.h>
#include <e32property.h>
#include <lbs.h>
#include <utf.h>

#include "clogimpl.h"

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "php_log.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void create_array( zval* array, const CLogEvent& aLogEvent );

const TUid KPositionCategory = { 0x2000E483 };

_LIT( KUnixEpochStr, "19700101:000000.000000" );

const TTime KUnixEpoch = TTime( KUnixEpochStr );

enum TPositionKeys
{
    EKeyLatitude = 1,
    EKeyLongitude,
    EKeyAltitude,
    EKeyAccuracyHoriz,
    EKeyAccuracyVert,
    EKeyPositionHistory,
    EKeyPosition
};

BEGIN_MODULE_FUNCTIONS(s60_log)
    PHP_FE( first_entry, NULL )
    PHP_FE( next_entry, NULL )
    PHP_FE( last_entry, NULL )
    PHP_FE( prev_entry, NULL )
    PHP_FE( set_filter, NULL )
END_MODULE_FUNCTIONS()

DEFINE_MODULE(s60_log, "1.0", PHP_MINIT(s60_log), NULL, NULL, NULL)

CLogImpl* iLogImpl;

PHP_MINIT_FUNCTION(s60_log)
{
    iLogImpl = CLogImpl::NewL();
    
    REGISTER_LONG_CONSTANT("S60_LOG_CALL_EVENT_TYPE", KLogCallEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_DATA_EVENT_TYPE", KLogDataEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_FAX_EVENT_TYPE", KLogFaxEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_SMS_EVENT_TYPE", KLogShortMessageEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_MAIL_EVENT_TYPE", KLogMailEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_TASK_SCHEDULER_EVENT_TYPE", KLogTaskSchedulerEventType, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("S60_LOG_PACKET_DATA_EVENT_TYPE", KLogPacketDataEventType, CONST_CS | CONST_PERSISTENT);
#if defined(KLogLbsSelfLocateEventType)
    REGISTER_LONG_CONSTANT("S60_LOG_LBS_SELF_LOCATE_EVENT_TYPE", KLogLbsSelfLocateEventType, CONST_CS | CONST_PERSISTENT);
#endif
#if defined(KLogLbsExternalLocateEventType)
    REGISTER_LONG_CONSTANT("S60_LOG_EXTERNAL_LOCATE_EVENT_TYPE", KLogLbsExternalLocateEventType, CONST_CS | CONST_PERSISTENT);
#endif
#if defined(KLogLbsTransmitLocationEventType)
    REGISTER_LONG_CONSTANT("S60_LOG_TRANSMIT_EVENT_TYPE", KLogLbsTransmitLocationEventType, CONST_CS | CONST_PERSISTENT);
#endif
#if defined(KLogLbsNetworkLocateEventType)
    REGISTER_LONG_CONSTANT("S60_LOG_NETWORK_LOCATE_EVENT_TYPE", KLogLbsNetworkLocateEventType, CONST_CS | CONST_PERSISTENT);
#endif
#if defined(KLogLbsAssistanceDataEventType)
    REGISTER_LONG_CONSTANT("S60_LOG_ASSISTANCE_EVENT_TYPE", KLogLbsAssistanceDataEventType, CONST_CS | CONST_PERSISTENT);
#endif

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(s60_log)
{
    delete iLogImpl;
    iLogImpl = NULL;    
    return 0;
}

void create_array( zval* return_value, const CLogEvent& logEvent )
{
    array_init(return_value); 
    
    add_assoc_long(return_value, "id", logEvent.Id() ); 
    add_assoc_long(return_value, "type", logEvent.EventType().iUid ); 
    add_assoc_long(return_value, "contact_item_id", logEvent.Contact() );
    add_assoc_long(return_value, "link", logEvent.Link() );
    
    HBufC8* dataZ = HBufC8::NewL( logEvent.Data().Length() + 1 );
    dataZ->Des().Copy( logEvent.Data() );   
    add_assoc_string(return_value, "data", (char*)dataZ->Des().PtrZ(), 1 );
    delete dataZ;
    
    TRAPD( err, 
    {
        HBufC8* desc = HBufC8::NewL( logEvent.Description().Length() * 2 + 1 );
        
        TPtr8 descPtr = desc->Des();
        
        CnvUtfConverter::ConvertFromUnicodeToUtf8( descPtr, logEvent.Description() );
        
        if( desc )
        {
            add_assoc_string(return_value, "description", (char*)desc->Des().PtrZ(), 1 );
        }
        
        delete desc;
    } );
    
    TRAPD( err2, 
    {
        HBufC8* direction = HBufC8::NewL( logEvent.Direction().Length() * 2 + 1 );
        
        TPtr8 dirPtr = direction->Des();
        
        CnvUtfConverter::ConvertFromUnicodeToUtf8( dirPtr, logEvent.Direction() );
        
        if( direction )
        {
            add_assoc_string(return_value, "direction", (char*)direction->Des().PtrZ(), 1 );
        }
        
        delete direction;
    } );
    
    TRAPD( err3, 
    {
        HBufC8* subject = HBufC8::NewL( logEvent.Subject().Length() * 2 + 1 );
        
        TPtr8 subjPtr = subject->Des();
        
        CnvUtfConverter::ConvertFromUnicodeToUtf8( subjPtr, logEvent.Subject() );

        if( subject )
        {
            add_assoc_string(return_value, "subject", (char*)subject->Des().PtrZ(), 1 );
        }
        
        delete subject;
    } );
    
    TRAPD( err4, 
    {
        HBufC8* remote_party = HBufC8::NewL( logEvent.RemoteParty().Length() * 2 + 1 );
        
        TPtr8 rpPtr = remote_party->Des();
        
        CnvUtfConverter::ConvertFromUnicodeToUtf8( rpPtr, logEvent.RemoteParty() );

        if( remote_party )
        {
            add_assoc_string(return_value, "remote_party", (char*)remote_party->Des().PtrZ(), 1 );
        }
        
        delete remote_party;
    } );
    
    TTime time = logEvent.Time();
    
    add_assoc_double(return_value, "time", convert_utc_time_to_utc_real( time ) );
    add_assoc_long(return_value, "duration", logEvent.Duration() );
    add_assoc_long(return_value, "duration_type", logEvent.DurationType() );
    
    TBuf8<255> phoneNbr;
    phoneNbr.Copy( logEvent.Number().Left(255) );
    add_assoc_string(return_value, "phone_number", (char*)phoneNbr.PtrZ(), 1 );
}


/**
*   FUNCTIONS
*
*/
PHP_FUNCTION( first_entry )
{
    GET_FUNCTION_ARGS_0( );
    
    TRAPD( err, 
    {
        const CLogEvent& logEvent = iLogImpl->FirstEntryL();
        create_array( return_value, logEvent );
    } );
    
    if( err )
    {
        RETURN_SYMBIAN_ERROR( err );
    }
}

PHP_FUNCTION( next_entry )
{
    GET_FUNCTION_ARGS_0();
    
    TRAPD( err, 
    {
        const CLogEvent& logEvent = iLogImpl->NextEntryL();
        create_array( return_value, logEvent );
    } );
    
        if( err )
    {
        RETURN_SYMBIAN_ERROR( err );
    }
    
}

PHP_FUNCTION( last_entry )
{
    GET_FUNCTION_ARGS_0();
    
    TRAPD( err, 
    {
        const CLogEvent& logEvent = iLogImpl->LastEntryL();
        create_array( return_value, logEvent );
    } );
    
    if( err )
    {
        RETURN_SYMBIAN_ERROR( err );
    }
}

PHP_FUNCTION( prev_entry )
{
    GET_FUNCTION_ARGS_0();
    
    TRAPD( err, 
    {
        const CLogEvent& logEvent = iLogImpl->PrevEntryL();
        create_array( return_value, logEvent );
    } );
    
    if( err )
    {
        RETURN_SYMBIAN_ERROR( err );
    }
}

PHP_FUNCTION( set_filter )
{
    GET_FUNCTION_ARGS_0();
    
    TRAPD( err, 
    {
        iLogImpl->SetFilterL();
    } );
    
    if( err )
    {
        RETURN_SYMBIAN_ERROR( err );
    }
}

}

CLogImpl* CLogImpl::NewL()
{
    CLogImpl* self = new(ELeave)CLogImpl();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
}


CLogImpl::CLogImpl() 
{
    
}

CLogImpl::~CLogImpl()
{
    iFsSession.Close();
    
    delete iLogClient;
    delete iLogView;
    delete iLogFilter;
}

void CLogImpl::ConstructL()
{
    CActiveScheduler::Replace( new CActiveScheduler() );
    
    User::LeaveIfError( iFsSession.Connect() );
    
    iLogClient = CLogClient::NewL( iFsSession );
    iLogView = CLogViewEvent::NewL( *iLogClient );
    iLogFilter = CLogFilter::NewL();
    
    
}
    

const CLogEvent& CLogImpl::FirstEntryL()
{
    CWaitAndReturn* wait = new (ELeave)CWaitAndReturn();
    CleanupStack::PushL( wait );
    
    if( iLogView->FirstL( wait->iStatus ) )
    {
        TInt status = wait->Wait();
        User::LeaveIfError( status );
    }
    else
    {
        User::Leave( KErrNotReady );
    }
    
    CleanupStack::PopAndDestroy(); // wait
    
    return iLogView->Event();
}

const CLogEvent& CLogImpl::NextEntryL()
{
    CWaitAndReturn* wait = new (ELeave)CWaitAndReturn();
    CleanupStack::PushL( wait );
    
    if( iLogView->NextL( wait->iStatus ) )
    {
        TInt status = wait->Wait();
        
        User::LeaveIfError( status );
    }
    else
    {
        User::Leave( KErrNotReady );
    }
    
    
    CleanupStack::PopAndDestroy(); // wait
    
    return iLogView->Event();
}

const CLogEvent& CLogImpl::LastEntryL()
{
    CWaitAndReturn* wait = new (ELeave)CWaitAndReturn();
    CleanupStack::PushL( wait );
    
    if( iLogView->LastL( wait->iStatus ) )
    {
        TInt status = wait->Wait();
        User::LeaveIfError( status );
    }
    else
    {
        User::Leave( KErrNotReady );
    }
    
    
    CleanupStack::PopAndDestroy(); // wait
    
    return iLogView->Event();

}

const CLogEvent& CLogImpl::PrevEntryL()
{
    CWaitAndReturn* wait = new (ELeave)CWaitAndReturn();
    
    CleanupStack::PushL( wait );
    
    if( iLogView->PreviousL( wait->iStatus ) )
    {
        TInt status = wait->Wait();
        
        User::LeaveIfError( status );
    }
    else
    {
        User::Leave( KErrNotReady );
    }
    
    CleanupStack::PopAndDestroy(); // wait
    
    return iLogView->Event();
}

void CLogImpl::SetFilterL()
{
    CWaitAndReturn* wait = new (ELeave)CWaitAndReturn();
    
    CleanupStack::PushL( wait );
    
    if( iLogView->SetFilterL( *iLogFilter, wait->iStatus ) )
    {
        TInt status = wait->Wait();
        User::LeaveIfError( status );
    }
    else
    {
        User::Leave( KErrNotReady );
    }
    
    CleanupStack::PopAndDestroy(); // wait
}


CWaitAndReturn::CWaitAndReturn() : CActive( CActive::EPriorityStandard )
{
    CActiveScheduler::Add( this );
}

CWaitAndReturn::~CWaitAndReturn()
{
}

TInt CWaitAndReturn::Wait()
{
    SetActive();
    
    iWait = new(ELeave)CActiveSchedulerWait();
    iWait->Start();
    
    TInt status = iStatus.Int();
    
    return status;
}

void CWaitAndReturn::RunL()
{
    iWait->AsyncStop();
    delete iWait;
    iWait = NULL;
}

void CWaitAndReturn::DoCancel()
{
    iWait->AsyncStop();
    
    delete iWait;
    iWait = NULL;
}
