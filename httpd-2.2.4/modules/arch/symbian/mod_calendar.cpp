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
#if defined(EKA2)
#include <calsession.h>
#include <calinstance.h>
#include <calinstanceview.h>
#include <calentry.h>
#include <calprogresscallback.h>
#else
#include <agclient.h>
#include <agmmodel.h>
#include <agmentry.h>
#include <agmdate.h>
#endif

#include "symbian_mod_utils.h"

// Apache httpd
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"


/* Calendar Module - displays the calendar on a Symbian phone.
 *
 * To enable this, add the following lines into any config file:
 *
 * <Location /calendar>
 * SetHandler calendar
 * </Location>
 *
 * You may want to protect this location by password or domain so no one
 * else can look at it.  Then you can access the contacts an URL like:
 *
 * http://your_server_name/calendar
 *
 * /calendar   - 
 *
 */


#if !defined(EKA2)
namespace
{
    _LIT(KCalendarFileName, "C:\\System\\Data\\Calendar");
}
#endif


namespace
{
    const TInt KDateKeySize = 8; // strlen("20060101");
}


namespace
{
    void ChangeToFirstDayOfWeek(TTime& aTime)
        {
        aTime -= TTimeIntervalDays(aTime.DayNoInWeek());
        }
    

    void ChangeToFirstDayOfMonth(TTime& aTime)
        {
        aTime -= TTimeIntervalDays(aTime.DayNoInMonth());
        }

    
    void PutDay(const TTime& aTime, request_rec* r)
        {
        TDateTime 
            time = aTime.DateTime();
        
        _LIT(dot, ".");

        TBuf8<6>  // strlen("30.12.");
            text;
        
        text.AppendNum(time.Day() + 1);
        text.Append(dot);
        text.AppendNum(static_cast<TInt>(time.Month()) + 1);
        text.Append(dot);

        ap_rputd(text, r);
        }


    void ParseViewTypeAndStartDateL(char*  args,
                                    TChar& viewType,
                                    TTime& firstDay)
        {
        TLex8 
            lexer(reinterpret_cast<const TUint8*>(args));
        
        _LIT8(keyView, "view");
        _LIT8(keyStart, "start");

        TBuf8<5> // strlen("start");
            key;
        
        TInt 
            count = 0;
        
        do
            {
            lexer.Mark();
            
            while (!lexer.Eos() && lexer.Peek() != '=')
                {
                lexer.Inc();
                }
            
            if (!lexer.Eos() && lexer.Peek() == '=')
                {
                TPtrC8
                    token = lexer.MarkedToken();

                if (token.Length() > key.MaxLength())
                    User::Leave(KErrArgument);
                
                key = token;
                
                lexer.Inc();
                lexer.Mark();
                }
                
            if (key.Compare(keyView) == 0 && !lexer.Eos())
                {
                viewType = lexer.Get();
                
                lexer.Mark();
                }
            else if (key.Compare(keyStart) == 0)
                {
                TPtrC8
                    dateKey = lexer.Remainder();

                if (dateKey.Length() != KDateKeySize)
                    User::Leave(KErrArgument);
                
                TBuf<KDateKeySize + 1>
                    date;
                
                date.Copy(dateKey);
                date.Append(_L(":"));
                
                firstDay.Set(date);
                }
            
            if(lexer.Peek() == '&')
                lexer.Inc();
            
            }
        while(++count < 2);
        }


    _LIT(KZero, "0");

    
    void GetDateKeyL(const TTime& aTime, TDes8& aKey)
        {
        aKey.Zero();
        
        TDateTime
            date = aTime.DateTime();
        
        TInt
            year = date.Year();
        
        if (year < 0)
            User::Leave(KErrArgument);
        
        if (year < 1000)
            aKey.Append(KZero);
        
        if (year < 100)
            aKey.Append(KZero);
        
        if (year < 10)
            aKey.Append(KZero);
        
        aKey.AppendNum(year);
        
        TInt
            month = date.Month();
        
        if (month < 10)
            aKey.Append(KZero);
        
        aKey.AppendNum(month);
        
        TInt
            day = date.Day();
        
        if (day < 10)
            aKey.Append(KZero);
        
        aKey.AppendNum(day);
        }


    void GenerateWeekHeaderL(const TTime& aCurrent, request_rec* r)
        {
        TTime
            prev(aCurrent - TTimeIntervalDays(7)),
            next(aCurrent + TTimeIntervalDays(7));
            
        TBuf8<KDateKeySize> 
            tmp;
            
        tmp.Zero();
        tmp.AppendNum(aCurrent.WeekNoInYear());

        ap_rputs("<table border=\"1\">\n", r);
        ap_rputs("<tr><td><b>Week ", r);
        ap_rputd(tmp, r);
            
        tmp.Zero();
        tmp.AppendNum(aCurrent.DateTime().Year());

        ap_rputs("</b></td><td><b>", r);
        ap_rputd(tmp, r);
		
        ap_rputs("</b></td></tr><tr><td>"
                 "<a href=\"calendar?view=W&start=", r);
		
        GetDateKeyL(prev, tmp);
        
        ap_rputd(tmp, r);
		
        ap_rputs("\">Previous Week</a></td><td>"
                 "<a href=\"calendar?view=M&start=", r);
		
        
        GetDateKeyL(aCurrent, tmp);

        ap_rputd(tmp, r);
		
        ap_rputs("\">Month View</a></td><td>"
                 "<a href=\"calendar?view=W&start=", r);
		
        GetDateKeyL(next, tmp);
        
        ap_rputd(tmp, r);
		
        ap_rputs("\">Next Week</a></td></tr>\n", r);
        }


    void GenerateMonthHeaderL(const TTime& aCurrent, request_rec* r)
        {
        TTime
            prev(aCurrent - TTimeIntervalMonths(1)),
            next(aCurrent + TTimeIntervalMonths(1));
        
        TBuf8<KDateKeySize>
            tmp;
		
        ap_rputs("<table border=\"1\">\n", r);

        // Link to previous month.
        ap_rputs("<tr><td width=\"14%\">"
                 "<a href=\"calendar?view=M&start=", r);

        GetDateKeyL(prev, tmp);

        ap_rputd(tmp, r);
		
        // This month and year.
        ap_rputs("\">Previous Month</a></td>"
                 "<td colspan=\"5\"  width=\"70%\"><b> ", r);
        ap_rputdL(TMonthName(aCurrent.DateTime().Month()), r);
        ap_rputs(" ", r);

        tmp.Zero();
        tmp.AppendNum(aCurrent.DateTime().Year());
        ap_rputd(tmp, r);
		
        // Link to next month.
        ap_rputs(" </b></td><td width=\"14%\">"
                 "<a href=\"calendar?view=M&start=", r);
		
        GetDateKeyL(next, tmp);

        ap_rputd(tmp, r);
		
        ap_rputs("\">Next Month</a></td></tr>\n", r);
		
        /* Put week day names on the top of the table */
        ap_rputs("<tr>\n", r);
        for(TInt day = 0; day < 7; day++)
            {
            ap_rputs(" <td width=\"14%\"><b>", r);
            ap_rputdL(TDayName(static_cast<TDay>(day)), r);
            ap_rputs("</b></td>\n", r);
            }
        ap_rputs("</tr>\n", r);
        }


    void GenerateMonthWeekHeader(const TTime& aDay,
                                 const TTime& aFirst,
                                 const TTime& aLast,
                                 request_rec* r)
        {
        ap_rputs("<tr>\n", r);
        
        TTime
            day(aDay);
        
        for(TInt i = 0; i < 7; ++i)
            {
            ap_rputs("<td width=\"14%\"><b>", r);

            if (day >= aFirst && day <= aLast)
                PutDay(day, r);
            
            ap_rputs("</b></td>\n", r);
            
            day += TTimeIntervalDays(1);
            }
        
        ap_rputs("</tr>\n", r);
        } 


    _LIT8(KAnniversary, "Anniversary");
    _LIT8(KMeeting,     "Meeting");
    _LIT8(KMemo,        "Memo");
    _LIT8(KReminder,    "Reminder");
    _LIT8(KTodo,        "Todo");

    _LIT8(KOccasion,    "Occasion: ");
    _LIT8(KSubject,     "Subject: ");
    

    void GenerateEntryBodyL(const TDesC8& aKind,
                            const TDesC8& aHeader,
                            const TDesC&  aText,
                            request_rec*  r)
        {
        ap_rputs("<table>\n"
                 "  <tr>\n"
                 "    <td><b>", r);      
        ap_rputd(aKind, r); ap_rputs("</b></td>\n", r);
        ap_rputs("  </tr>\n"
                 "    <td><i>", r);
        ap_rputd(aHeader, r), ap_rputs("</i>", r);
        ap_rputdL(aText, r); ap_rputs("</td>\n", r);
        ap_rputs("  </tr>\n"
                 "</table>\n", r);
        }
}


#if defined(EKA2)

namespace
{
    void GenerateEntryBodyL(CCalEntry&    aEntry,
                            const TDesC8& aKind,
                            const TDesC8& aHeader,
                            request_rec*  r)
        {
        GenerateEntryBodyL(aKind, aHeader, aEntry.SummaryL(), r);
        }

    
    void GenerateAnnivL(CCalEntry& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KAnniversary, KOccasion, r);
        }


    void GenerateApptL(CCalEntry& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KMeeting, KSubject, r);
        }


    void GenerateEventL(CCalEntry& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KMemo, KSubject, r);
        }


    void GenerateTodoL(CCalEntry& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KTodo, KSubject, r);
        }


    //
    // This is not possible to create on the phone.
    // 

    void GenerateReminderL(CCalEntry& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KReminder, KSubject, r);
        }


    void GenerateEntryL(CCalInstance& aInstance,
                        const TDesC8& aHeader,
                        const TDesC8& aFooter,
                        request_rec*  r)
        {
        // No __UHEAP_MARKs here. aInstance allocates stuff that is
        // released only later.

        CCalEntry
            &entry = aInstance.Entry();

        if (aHeader.Size())
            ap_rputd(aHeader, r);
            
        switch (entry.EntryTypeL())
            {
            case CCalEntry::EAnniv:
                GenerateAnnivL(entry, r);
                break;                        

            case CCalEntry::EAppt:
                GenerateApptL(entry, r);
                break;
                    
            case CCalEntry::EEvent:
                GenerateEventL(entry, r);
                break;
                    
            case CCalEntry::EReminder:
                GenerateReminderL(entry, r);
                break;                        

            case CCalEntry::ETodo:
                // Todos should really be shown on the current data,
                // but we'll just ignore that for now. This is but a
                // concept demo.
                GenerateTodoL(entry, r);
                break;
            }
            
        if (aFooter.Size())
            ap_rputd(aFooter, r);
        }


    TInt GenerateWeekDayEventsL(CCalInstanceView&                  aView,
                                const TTime&                       aDay,
                                const RPointerArray<CCalInstance>& aInstances,
                                TInt                               aIndex,
                                request_rec*                       r)
        {
        TInt
            i = aIndex;
        TBool
            exhausted = EFalse;
        
        while ((i < aInstances.Count()) && !exhausted)
            {
            CCalInstance
                &instance = *aInstances[i];
            
            if (aDay.DayNoInWeek() == 
                instance.Time().TimeLocalL().DayNoInWeek())
                {
                _LIT8(KHeader, "<tr><td></td><td colspan=\"2\">");
                _LIT8(KFooter, "</td></tr>");
                
                GenerateEntryL(instance, KHeader, KFooter, r);
                
                ++i;
                }
            else
                exhausted = ETrue;
            }
        
        return i;
        }
            
            
    void GenerateWeekViewL(CCalInstanceView&                  aView,
                           const TTime&                       aBegin,
                           const RPointerArray<CCalInstance>& aInstances,
                           request_rec*                       r)
        {
        GenerateWeekHeaderL(aBegin, r);
            
        TTime
            day(aBegin);
        TInt
            j = 0;
            
        for(TInt i = 0; i < 7; i++)
            {
            ap_rputs("<tr><td>", r); 
            ap_rputdL(TDayName(day.DayNoInWeek()), r);
            ap_rputs("</td><td colspan=\"2\">", r);

            PutDay(day, r);

            ap_rputs("</td></tr>\n", r);
			
            if (j < aInstances.Count())
                {
                CCalInstance
                    &instance = *aInstances[j];
                
                if (instance.Time().TimeLocalL().DayNoInWeek() == 
                    day.DayNoInWeek())
                    j = GenerateWeekDayEventsL(aView, day, aInstances, j, r);
                }
                
            ap_rputs("<tr><td colspan=\"3\"></td></tr>\n", r); 	
            
            day += TTimeIntervalDays(1);
            }

        ap_rputs("<table>\n", r);
        }


    void GenerateWeekViewL(CCalInstanceView& aView,
                           const TTime&      aBegin,
                           request_rec*      r)
        {
        // No __UHEAP_MARK/__UHEAP_MARKEND here. Memory is allocated
        // by aView and freed only later.
            
        RPointerArray<CCalInstance>
            instances;
        
        CleanupClosePushL(instances);
        
        TCalTime
            begin, end;

        begin.SetTimeLocalL(aBegin);
        end.SetTimeLocalL(TTime(aBegin) += TTimeIntervalDays(7));

        CalCommon::TCalTimeRange
            range(begin, end);
                    
        aView.FindInstanceL(instances, CalCommon::EIncludeAll, range);

        GenerateWeekViewL(aView, aBegin, instances, r);

        for (TInt i = 0; i < instances.Count(); ++i)
            delete instances[i];

        CleanupStack::PopAndDestroy(); // instances.Close();
        }


    TInt GenerateMonthDayEventsL(CCalInstanceView&                  aView,
                                 const TTime&                       aDay,
                                 const RPointerArray<CCalInstance>& aInstances,
                                 TInt                               aIndex,
                                 request_rec*                       r)
        {
        TInt
            i = aIndex;
        TBool
            exhausted = EFalse;
        
        while ((i < aInstances.Count()) && !exhausted)
            {
            CCalInstance
                &instance = *aInstances[i];
            
            if (aDay.DayNoInWeek() == 
                instance.Time().TimeLocalL().DayNoInWeek())
                {
                _LIT8(KHeader, "");
                _LIT8(KFooter, "");

                GenerateEntryL(instance, KHeader, KFooter, r);
                
                ++i;
                }
            else
                exhausted = ETrue;
            }
        
        return i;
        }
            
            
    void GenerateMonthViewL(CCalInstanceView&                  aView,
                            const TTime&                       aFirst,
                            const RPointerArray<CCalInstance>& aInstances,
                            request_rec*                       r)
        {
        GenerateMonthHeaderL(aFirst, r);
            
        TTime
            first(aFirst),
            last(aFirst + TTimeIntervalDays(aFirst.DaysInMonth() - 1)),
            begin(aFirst - TTimeIntervalDays(aFirst.DayNoInWeek())),
            end(last + TTimeIntervalDays(7 - last.DayNoInWeek())),
            day(begin);
        TInt
            i = 0;
        
        do
            {
            GenerateMonthWeekHeader(day, first, last, r);
            
            /* And then entries of those dates. */
            ap_rputs("<tr>\n", r);
            
            for (TInt j = 0; j < 7; j++)
                {
                ap_rputs("<td width=\"14%\">", r);
                
                if ((day >= first) && (day <= last))
                    {
                    TBool
                        events = EFalse;
                    
                    if (i < aInstances.Count())
                        {
                        CCalInstance
                            &instance = *aInstances[i];
                        
                        if (instance.Time().TimeLocalL().DayNoInWeek() == 
                            day.DayNoInWeek())
                            {
                            i = GenerateMonthDayEventsL(aView, day, 
                                                        aInstances, i, r);

                            events = ETrue;
                            }
                        }
                
                    if (!events)
                        ap_rputs(" - ", r);
                    }

                ap_rputs("</td>\n", r);
                
                day += TTimeIntervalDays(1);
                }

            ap_rputs("</tr>\n", r);
            }
        while (day < end);

        ap_rputs("<table>\n", r);
        }


    void GenerateMonthViewL(CCalInstanceView& aView,
                            const TTime&      aBegin,
                            request_rec*      r)
        {
        // No __UHEAP_MARK/__UHEAP_MARKEND here. Memory is allocated
        // by aView and freed only later.
            
        RPointerArray<CCalInstance>
            instances;
        
        CleanupClosePushL(instances);
        
        TCalTime
            begin, end;

        begin.SetTimeLocalL(aBegin);
        end.SetTimeLocalL(TTime(aBegin) += TTimeIntervalMonths(1));

        CalCommon::TCalTimeRange
            range(begin, end);
                    
        aView.FindInstanceL(instances, CalCommon::EIncludeAll, range);

        GenerateMonthViewL(aView, aBegin, instances, r);

        for (TInt i = 0; i < instances.Count(); ++i)
            delete instances[i];

        CleanupStack::PopAndDestroy(); // instances.Close();
        }


    void GenerateCalendarL(CCalInstanceView& aView, request_rec* r)
        {
        TChar 
            viewType = '\0';
        TTime
            start;

        if(r->args)
            ParseViewTypeAndStartDateL(r->args, viewType, start);	

        if (viewType == 'W')
            {
            ChangeToFirstDayOfWeek(start);
            
            GenerateWeekViewL(aView, start, r);
            }
        else if(viewType == 'M')
            {
            ChangeToFirstDayOfMonth(start);			
            
            GenerateMonthViewL(aView, start, r);
            }
        else/* default and */
            {
            TTime
                current;
            
            current.HomeTime();
            
            TDateTime
                now = current.DateTime();
            
            now.SetHour(0);
            now.SetMinute(0);
            now.SetSecond(0);
            
            start = now;
            
            ChangeToFirstDayOfWeek(start);

            GenerateWeekViewL(aView, start, r);
            }

        }

    
    class CProgress : public CBase,
                      public MCalProgressCallBack
        {
    public:
        ~CProgress()
            {
            delete ipScheduler;
            }

        static CProgress* NewL()
            {
            CProgress
                *pThis = NewLC();
            
            CleanupStack::Pop(pThis);

            return pThis;
            }

        static CProgress* NewLC()
            {
            CProgress
                *pThis = new (ELeave) CProgress;
            
            CleanupStack::PushL(pThis);
            pThis->ConstructL();

            return pThis;
            }

        
        void Wait()
            {
            ipScheduler->Start();
            }

    private:
        void ConstructL()
            {
            ipScheduler = new (ELeave) CActiveSchedulerWait;
            }
        
        void Progress(TInt /*aPercentageCompleted*/)
            {
            }


        void Completed(TInt /*aError*/)
            {
            ipScheduler->AsyncStop();
            }

        TBool NotifyProgress()
            {
            return EFalse;
            }

    private:
        CActiveSchedulerWait* ipScheduler;
        };
    

    int GenerateCalendarL(request_rec* r)
        {
        CCalSession
            *pSession = CCalSession::NewL();
        
        CleanupStack::PushL(pSession);
        
        pSession->OpenL(KNullDesC);

        CProgress
            *pCallback = CProgress::NewLC();
        
        CCalInstanceView
            *pView = CCalInstanceView::NewL(*pSession, *pCallback);

        CleanupStack::PushL(pView);

        pCallback->Wait();
        
        GenerateCalendarL(*pView, r);
        
        CleanupStack::PopAndDestroy(pView);
        CleanupStack::PopAndDestroy(pCallback);
        CleanupStack::PopAndDestroy(pSession);

        return OK;
        }
}

#else // EKA2

namespace
{
    void GenerateEntryBodyL(const TDesC8&    aKind,
                            const TDesC8&    aHeader,
                            const CRichText& aText,
                            request_rec*     r)
        {
        HBufC
            *pBuf = HBufC::NewLC(aText.DocumentLength());
        TPtr
            des = pBuf->Des();
        
        aText.Extract(des);
        
        GenerateEntryBodyL(aKind, aHeader, des, r);
        
        CleanupStack::PopAndDestroy(pBuf);
        }
        

    void GenerateEntryBodyL(CAgnEntry&    aEntry,
                            const TDesC8& aKind,
                            const TDesC8& aHeader,
                            request_rec*  r)
        {
        GenerateEntryBodyL(aKind, aHeader, *aEntry.RichTextL(), r);
        }

    
    void GenerateEntryL(CAgnAnniv& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KAnniversary, KOccasion, r);
        }


    void GenerateEntryL(CAgnAppt& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KMeeting, KSubject, r);
        }


    void GenerateEntryL(CAgnEvent& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KMemo, KSubject, r);
        }


    void GenerateEntryL(CAgnTodo& aEntry, request_rec* r)
        {
        GenerateEntryBodyL(aEntry, KTodo, KSubject, r);
        }


    void GenerateEntryL(RAgendaServ&    aServer, 
                        CAgnEntryModel& aModel,
                        const TDesC8&   aHeader,
                        const TDesC8&   aFooter,
                        request_rec*    r)
        {
        __UHEAP_MARK;
            
        CAgnSortEntryAllocator
            *pAllocator = aModel.SortEntryAllocator();
        CAgnSortEntry 
            *pSortEntry = aServer.DateIteratorCurrentElement(pAllocator);
        CAgnEntry 
            *pEntry = aModel.FetchEntryL(pSortEntry->Id());
                    
        CleanupStack::PushL(pEntry);
            
        aModel.DeleteLiteEntry(pSortEntry);
            
        if (aHeader.Size())
            ap_rputd(aHeader, r);
            
        switch (pEntry->Type())
            {
            case CAgnEntry::EAppt:
                GenerateEntryL(*pEntry->CastToAppt(), r);
                break;
                    
            case CAgnEntry::ETodo:
                GenerateEntryL(*pEntry->CastToTodo(), r);
                break;
                    
            case CAgnEntry::EEvent:
                GenerateEntryL(*pEntry->CastToEvent(), r);
                break;
                    
            case CAgnEntry::EAnniv:
                GenerateEntryL(*pEntry->CastToAnniv(), r);
                break;                        
            }
            
        if (aFooter.Size())
            ap_rputd(aFooter, r);
            
        CleanupStack::PopAndDestroy(pEntry);

        __UHEAP_MARKEND;
        }


    TBool GenerateWeekDayEventsL(RAgendaServ&    aServer, 
                                 CAgnEntryModel& aModel,
                                 TAgnDate        aDay,
                                 request_rec*    r)
        {
        TBool
            exhausted = EFalse;
        TAgnDate 
            key = aServer.DateIteratorCurrentKey();
            
        while ((key <= aDay) && !exhausted)
            {
            if (key == aDay)
                {
                _LIT8(KHeader, "<tr><td></td><td colspan=\"2\">");
                _LIT8(KFooter, "</td></tr>");
                    
                GenerateEntryL(aServer, aModel, KHeader, KFooter, r);
                }
                
            aServer.DateIteratorNext();
                
            exhausted = aServer.DateIteratorAtEnd();
                
            if (!exhausted) 
                key = aServer.DateIteratorCurrentKey();
            }
            
        return exhausted;
        }
            
            
    void GenerateWeekViewL(RAgendaServ&    aServer, 
                           CAgnEntryModel& aModel,
                           const TTime&    aBegin,
                           request_rec*    r)
        {
        __UHEAP_MARK;
            
        GenerateWeekHeaderL(aBegin, r);
            
        TAgnDate
            weekStart = TAgnDateTime(aBegin).Date();
        TAgnFilter
            filter;
        TTime 
            now;

        now.HomeTime();
            
        aServer.CreateDateIterator(now, &filter, weekStart);

        TBool 
            exhausted = aServer.DateIteratorAtEnd();

        TInt
            monday = weekStart,
            sunday = weekStart + 6;
            
        for(TInt i = monday; i <= sunday; i++)
            {
            TDayName 
                weekday(AgnDateTime::DayNoInWeek(i));
                
            ap_rputs("<tr><td>", r); 
            ap_rputdL(weekday, r);
            ap_rputs("</td><td colspan=\"2\">", r);

            PutDay(AgnDateTime::AgnDateToTTime(i), r);

            ap_rputs("</td></tr>\n", r);
			
            if(!exhausted)
                exhausted = GenerateWeekDayEventsL(aServer, aModel, i, r);
                
            ap_rputs("<tr><td colspan=\"3\"></td></tr>\n", r); 	
            }

        ap_rputs("<table>\n", r);

        __UHEAP_MARKEND;
        }


    void GenerateMonthDayEventsL(RAgendaServ&    aServer,
                                 CAgnEntryModel& aModel,
                                 const TTime&    aDay,
                                 request_rec*    r)
        {
        TBool
            exhausted = EFalse;
        
        while (!aServer.DateIteratorAtEnd() && !exhausted)
            {
            TAgnDate 
                key = aServer.DateIteratorCurrentKey();
            
            if (key == TAgnDateTime(aDay).Date())
                {
                _LIT8(KHeader, "");
                _LIT8(KFooter, "");

                GenerateEntryL(aServer, aModel, KHeader, KFooter, r); 
                
                aServer.DateIteratorNext();
                }
            else
                exhausted = ETrue;
            }
        }
    
        
    void GenerateMonthViewL(RAgendaServ&    aServer, 
                            CAgnEntryModel& aModel,
                            const TTime&    aFirst,
                            request_rec*    r)
        {
        GenerateMonthHeaderL(aFirst, r);
        
        TAgnDate
            monthStart = TAgnDateTime(aFirst).Date();
        TTime 
            now;
        
        now.HomeTime();

        TAgnFilter
            entryFilter;

        aServer.CreateDateIterator(now, &entryFilter, monthStart);
        
        TTime
            first(aFirst),
            last(aFirst + TTimeIntervalDays(aFirst.DaysInMonth() - 1)),
            begin(aFirst - TTimeIntervalDays(aFirst.DayNoInWeek())),
            end(last + TTimeIntervalDays(7 - last.DayNoInWeek())),
            day(begin);

        do
            {
            GenerateMonthWeekHeader(day, first, last, r);

            /* And then entries of those dates. */
            ap_rputs("<tr>\n", r);
            
            for (TInt j = 0; j < 7; j++)
                {	
                ap_rputs("<td width=\"14%\">", r);
		
                if ((day >= first) && (day <= last))
                    {
                    TBool
                        events = EFalse;
                    
                    if (!aServer.DateIteratorAtEnd())
                        {
                        TAgnDate
                            key = aServer.DateIteratorCurrentKey();
                        
                        if (key == TAgnDateTime(day).Date())
                            {
                            GenerateMonthDayEventsL(aServer, aModel, day, r);
                            
                            events = ETrue;
                            }
                        }
                    
                    if(!events)
                        ap_rputs(" - ", r);
                    }

                ap_rputs("</td>\n", r);
                
                day += TTimeIntervalDays(1);
                }

            ap_rputs("</tr>\n", r);
            }
        while (day < end);
        
        ap_rputs("<table>\n", r);
        }


    void GenerateCalendarL(RAgendaServ&    aServer, 
                           CAgnEntryModel& aModel,
                           request_rec*    r)
        {
        TChar 
            viewType = '\0';
        TTime
            start;

        if(r->args)
            ParseViewTypeAndStartDateL(r->args, viewType, start);	

        if (viewType == 'W')
            {
            ChangeToFirstDayOfWeek(start);
            
            GenerateWeekViewL(aServer, aModel, start, r);
            }
        else if(viewType == 'M')
            {
            ChangeToFirstDayOfMonth(start);
            
            GenerateMonthViewL(aServer, aModel, start, r);
            }
        else/* default and */
            {
            TTime
                current;

            current.HomeTime();

            TDateTime
                now = current.DateTime();
            
            now.SetHour(0);
            now.SetMinute(0);
            now.SetSecond(0);
            
            start = now;

            ChangeToFirstDayOfWeek(start);
            
            GenerateWeekViewL(aServer, aModel, start, r);
            }

        }


    int GenerateCalendarL(request_rec* r)
        {
        RAgendaServ
            *pServer = RAgendaServ::NewL();
        
        CleanupStack::PushL(pServer);
        
        User::LeaveIfError(pServer->Connect());

        CleanupClosePushL(*pServer);
        
        CAgnIndexedModel
            *pModel = CAgnIndexedModel::NewL(NULL);

        CleanupStack::PushL(pModel);
        
        pModel->SetServer(pServer);
        pModel->OpenL(KCalendarFileName,
                      0, 0, 0,
                      NULL, ETrue);

        GenerateCalendarL(*pServer, *pModel, r);
        
        CleanupStack::PopAndDestroy(pModel);
        CleanupStack::PopAndDestroy(); // *pServer
        CleanupStack::PopAndDestroy(pServer);

        return OK;
        }
}

#endif // EKA


namespace
{
    int GenerateContentL(request_rec* r)
        {
        return GenerateCalendarL(r);
        }
    
    
    int CalendarHandler(request_rec* r)
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
    int calendar_handler(request_rec* r) {
        int
            rc = DECLINED;
        
        if (strcmp(r->handler, "calendar") == 0) {
            ap_set_content_type(r, "text/html");
            ap_rputs(DOCTYPE_HTML_3_2
                     "<html><head>\n"
                     "<title>My Calendar</title>\n"
                     "</head>\n"
                     "<body>\n", r);
            
            rc = CalendarHandler(r);
            
            ap_rputs("</body>", r);
            ap_rputs("</html>", r);
        }
        
        return rc;
    }


    void register_hooks(apr_pool_t* p) {
        ap_hook_handler(calendar_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }
}


extern "C"
{
    module AP_MODULE_DECLARE_DATA symbian_calendar_module =
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
