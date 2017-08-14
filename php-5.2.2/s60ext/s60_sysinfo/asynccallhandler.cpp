/**
 * ====================================================================
 * asynccallhandler.cpp
 *
 * Copyright (c) 2006 Nokia Corporation
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
 * ====================================================================
 */

#include <e32base.h>
#include "AsyncCallHandler.h"

void CAsyncCallHandlerSysInfo::ConstructL()
{ 
  iWait=new (ELeave) CActiveSchedulerWait;
  CActiveScheduler::Add(this);
};

CAsyncCallHandlerSysInfo::CAsyncCallHandlerSysInfo(CTelephony& telephony):CActive(CActive::EPriorityStandard),iTelephony(telephony)
{
};

CAsyncCallHandlerSysInfo* CAsyncCallHandlerSysInfo::NewL(CTelephony& telephony)
{
  CAsyncCallHandlerSysInfo* self=new (ELeave) CAsyncCallHandlerSysInfo(telephony);
  CleanupStack::PushL(self);
  self->ConstructL();
  CleanupStack::Pop(self);
  return self;
};

void CAsyncCallHandlerSysInfo::DoCancel()
{
};

CAsyncCallHandlerSysInfo::~CAsyncCallHandlerSysInfo()
{
  Cancel();
  delete iWait;
};

void CAsyncCallHandlerSysInfo::RunL()
{
 iWait->AsyncStop();
};

void CAsyncCallHandlerSysInfo::GetSignalStrength(CTelephony::TSignalStrengthV1Pckg& signalStrength,TRequestStatus& status)
{
  iTelephony.GetSignalStrength(iStatus,signalStrength);
  this->SetActive();
  iWait->Start();
  status=iStatus;
}


void CAsyncCallHandlerSysInfo::GetPhoneId(CTelephony::TPhoneIdV1Pckg& phoneId,TRequestStatus& status)
{  
  iTelephony.GetPhoneId(iStatus,phoneId);
  this->SetActive();
  iWait->Start();
  status=iStatus;
}

void CAsyncCallHandlerSysInfo::GetBatteryInfo(CTelephony::TBatteryInfoV1Pckg& batteryInfo,TRequestStatus& status)
{
  iTelephony.GetBatteryInfo(iStatus,batteryInfo);
  this->SetActive();
  iWait->Start();
  status=iStatus;
}
