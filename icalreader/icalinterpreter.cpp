/*  This file is part of Kalendar-Reader.
Kalendar-Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kalendar-Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "icalinterpreter.h"
#include "parameter.h"

#include <QDebug>

IcalInterpreter::IcalInterpreter( QObject *parent )
    :
      QThread( parent )
{
}


void IcalInterpreter::readIcal( const ICalBody &inIcal )
{
    if( not inIcal.m_vEventComponents.isEmpty() )
    {
        for( const VEventComponent* component : inIcal.m_vEventComponents )
        {
            AppointmentBasics *basic = nullptr;
            QList<AppointmentAlarm*> alarmList;
            AppointmentRecurrence *recurrence = nullptr;
            if( eventHasUsableRRuleOrNone( component ) ) // usable for our appointment structure?
            {
                readEvent( component, basic, alarmList, recurrence );
                makeAppointment( basic, recurrence, alarmList );
            }
        }
    }
}

void IcalInterpreter::readEvent( const VEventComponent* &inVEventComponent,
                AppointmentBasics* &outAppBasics,
                QList<AppointmentAlarm*> &outAppAlarmList,
                AppointmentRecurrence* &outAppRecurrence )
{
    outAppBasics = new AppointmentBasics();
    bool haveRecurrence = false;

    for( const Property* p : inVEventComponent->m_properties )
    {
        if( p->m_type == Property::PT_DESCRIPTION )
        {
            outAppBasics->m_description = p->m_content;
            continue;
        }
        if( p->m_type == Property::PT_DTEND )
        {
            outAppBasics->m_dtEnd = p->m_contentDateTime;
            Parameter* pp;
            if( p->getParameterByType( Parameter::TZIDPARAM, pp ) )
            {
                if( pp->m_contentTimeZone.isValid() )
                    outAppBasics->m_dtEnd.setTimeZone( pp->m_contentTimeZone );
            }
            continue;
        }
        if( p->m_type == Property::PT_DTSTART )
        {
            outAppBasics->m_dtStart = p->m_contentDateTime;
            Parameter* pp;
            if( p->getParameterByType( Parameter::TZIDPARAM, pp ) )
            {
                if( pp->m_contentTimeZone.isValid() )
                    outAppBasics->m_dtStart.setTimeZone( pp->m_contentTimeZone );
            }
            continue;
        }
        if( p->m_type == Property::PT_RRULE )
        {
            if( not haveRecurrence )
            {
                outAppRecurrence = new AppointmentRecurrence();
                haveRecurrence = true;
            }
            readRecurrence( p, outAppRecurrence );
            continue;
        }
        if( p->m_type == Property::PT_SUMMARY )
        {
            outAppBasics->m_summary = p->m_content;
            continue;
        }
        if( p->m_type == Property::PT_UID )
        {
            outAppBasics->m_uid = p->m_content;
            continue;
        }
        if( p->m_type == Property::PT_EXDATE )
        {
            if( not haveRecurrence )
            {
                outAppRecurrence = new AppointmentRecurrence();
                haveRecurrence = true;
            }
            if( p->m_storageType == Property::PST_DATETIME )
                outAppRecurrence->m_exceptionDates.append( p->m_contentDateTime );
            else // PST_DATETIMELIST
                outAppRecurrence->m_exceptionDates.append( p->m_contentDateTimeList );
            continue;
        }
        if( p->m_type == Property::PT_RDATE )
        {
            qDebug() << " FIXME: RDATE unsupported in Appointment";
            continue;
        }
        if( p->m_type == Property::PT_SEQUENCE )
        {
            outAppBasics->m_sequence = p->m_contentInteger;
            continue;
        }
        if( p->m_type == Property::PT_TRANSP )
        {
            outAppBasics->m_busyFree =  p->m_contentTransparency == Property::TT_TRANSPARENT ?
                                AppointmentBasics::FREE : AppointmentBasics::BUSY;
            continue;
        }
    }
    for( const VAlarmComponent* alarm : inVEventComponent->m_vAlarmComponents )
    {
        AppointmentAlarm *appAlarm = new AppointmentAlarm();
        readAlarm( alarm, outAppBasics, appAlarm );
        outAppAlarmList.append( appAlarm );
    }
}


void IcalInterpreter::readAlarm( const VAlarmComponent* inVAlarmComponent,
                                 const AppointmentBasics* inAppBasics,
                                 AppointmentAlarm* &outAppAlarm )
{
    for( const Property *p : inVAlarmComponent->m_properties )
    {
        if( p->m_type == Property::PT_REPEAT )
        {
            outAppAlarm->m_repeatNumber = p->m_contentInteger;
            continue;
        }
        if( p->m_type == Property::PT_TRIGGER )
        {
            if( p->m_parameters.isEmpty() )
                outAppAlarm->m_alarmSecs = p->durationToSeconds();
            else
            {
                Parameter* pp;
                if( p->getParameterByType( Parameter::TRIGRELPARAM, pp ) )
                {
                    outAppAlarm->m_alarmSecs = p->durationToSeconds();
                    if( pp->m_contentTriggerRelParam == Parameter::TRP_END )
                        outAppAlarm->m_alarmSecs +=
                                inAppBasics->m_dtStart.secsTo( (inAppBasics->m_dtEnd ) );
                    // No need to check for Parameter::TRP_START

                }
                else if( p->getParameterByType( Parameter::VALUETYPEPARAM, pp ) )
                {
                    if( pp->m_contentValueType == Parameter::VT_DATE_TIME )
                    {
                        DateTime dtAlarm;
                        dtAlarm = pp->m_contentDateTime;
                        outAppAlarm->m_alarmSecs = inAppBasics->m_dtStart.secsTo( dtAlarm );
                    }
                    if( pp->m_contentValueType == Parameter::VT_DURATION )
                    {
                        outAppAlarm->m_alarmSecs = p->durationToSeconds();
                    }
                }
            }
            continue;
        }
        if( p->m_type == Property::PT_DURATION )
        {
            if( p->m_storageType == Property::PST_DURATION )
                outAppAlarm->m_pauseSecs = p->durationToSeconds();
        }
    }
}


void IcalInterpreter::readRecurrence( const Property* inRecurrenceProperty,
                                      AppointmentRecurrence* &outAppRecurrence  )
{
    int numberOfByRules = 0;
    // This is more or less just stupid translation between
    // Ical-consts and appointment const with just different names.
    // More, we limit shortest recurrence to daily.
    for( const Parameter* p : inRecurrenceProperty->m_parameters )
    {
        if( p->m_type == Parameter::RR_FREQ )
        {
            switch( p->m_contentFrequency )
            {
                case Parameter::F_DAILY:
                    outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_DAILY;
                break;
                case Parameter::F_MONTHLY:
                    outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_MONTHLY;
                break;
                case Parameter::F_WEEKLY:
                    outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_WEEKLY;
                break;
                case Parameter::F_YEARLY:
                    outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_YEARLY;
                break;
                default:    // we limit to the above cases. No shorter recurrence in this program.
                break;
            }
        }
        else if( p->m_type == Parameter::RR_UNTIL )
        {
            outAppRecurrence->m_until = p->m_contentDateTime;
        }
        else if( p->m_type == Parameter::RR_COUNT )
        {
            outAppRecurrence->m_count = p->m_contentInteger;
        }
        else if( p->m_type == Parameter::RR_INTERVAL )
        {
            outAppRecurrence->m_interval = p->m_contentInteger;
        }
        else if( p->m_type == Parameter::RR_BYDAY )
        {
            numberOfByRules++;
            for( Parameter::IcalWeekDayType wd : p->m_contentDayMap.uniqueKeys() )
            {
                AppointmentRecurrence::WeekDay weekDay = AppointmentRecurrence::WeekDay::WD_MO;
                switch( wd )
                {
                    case Parameter::IcalWeekDayType::WD_MO:
                        weekDay = AppointmentRecurrence::WeekDay::WD_MO;
                    break;
                    case Parameter::IcalWeekDayType::WD_TU:
                        weekDay = AppointmentRecurrence::WeekDay::WD_TU;
                    break;
                    case Parameter::IcalWeekDayType::WD_WE:
                        weekDay = AppointmentRecurrence::WeekDay::WD_WE;
                    break;
                    case Parameter::IcalWeekDayType::WD_TH:
                        weekDay = AppointmentRecurrence::WeekDay::WD_TH;
                    break;
                    case Parameter::IcalWeekDayType::WD_FR:
                        weekDay = AppointmentRecurrence::WeekDay::WD_FR;
                    break;
                    case Parameter::IcalWeekDayType::WD_SA:
                        weekDay = AppointmentRecurrence::WeekDay::WD_SA;
                    break;
                    case Parameter::IcalWeekDayType::WD_SU:
                        weekDay = AppointmentRecurrence::WeekDay::WD_SU;
                    break;
                    default:
                    break;
                }
                for( int val : p->m_contentDayMap.values( wd ) )
                {
                    outAppRecurrence->m_byDayMap.insert( weekDay, val );
                }
            }
        }
        else if( p->m_type == Parameter::RR_BYMONTHDAY )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMonthDayList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYYEARDAY )
        {
            numberOfByRules++;
            outAppRecurrence->m_byYearDayList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYWEEKNO )
        {
            numberOfByRules++;
            outAppRecurrence->m_byWeekNumberList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYMONTH )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMonthList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYHOUR )
        {
            numberOfByRules++;
            outAppRecurrence->m_byHourList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYMINUTE )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMinuteList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYSECOND )
        {
            numberOfByRules++;
            outAppRecurrence->m_bySecondList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_BYSETPOS )
        {
            numberOfByRules++;
            outAppRecurrence->m_bySetPosList = p->m_contentIntList;
        }
        else if( p->m_type == Parameter::RR_WKST)
        {
            numberOfByRules++;
            switch( p->m_contentWeekDay )
            {
                case Parameter::IcalWeekDayType::WD_MO:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_MO;
                break;
                case Parameter::IcalWeekDayType::WD_TU:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_TU;
                break;
                case Parameter::IcalWeekDayType::WD_WE:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_WE;
                break;
                case Parameter::IcalWeekDayType::WD_TH:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_TH;
                break;
                case Parameter::IcalWeekDayType::WD_FR:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_FR;
                break;
                case Parameter::IcalWeekDayType::WD_SA:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_SA;
                break;
                case Parameter::IcalWeekDayType::WD_SU:
                    outAppRecurrence->m_startWeekday = AppointmentRecurrence::WeekDay::WD_SU;
                break;
                default:
                break;
            }
        }
    } // end for
    if( numberOfByRules == 0 )
    {
        // these are the simple cases
        switch( outAppRecurrence->m_frequency )
        {
            case AppointmentRecurrence::RFT_DAILY:
                outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_SIMPLE_DAILY;
            break;
            case AppointmentRecurrence::RFT_MONTHLY:
                outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_SIMPLE_MONTHLY;
            break;
            case AppointmentRecurrence::RFT_WEEKLY:
                outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_SIMPLE_WEEKLY;
            break;
            case AppointmentRecurrence::RFT_YEARLY:
                outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_SIMPLE_YEARLY;
            break;
            default:    // we limit to the above cases. No shorter recurrence in this program.
            break;
        }
    }
}


bool IcalInterpreter::eventHasUsableRRuleOrNone( const VEventComponent* inVEventComponent )
{
    bool ret = true;
    for( const Property *p : inVEventComponent->m_properties )
    {
        if( p->m_type == Property::PT_RRULE )
        {
            Parameter* param;
            bool found = p->getParameterByType( Parameter::RR_FREQ, param );
            if( found and ( param->m_contentFrequency == Parameter::F_SECONDLY or
                param->m_contentFrequency == Parameter::F_MINUTELY or
                param->m_contentFrequency == Parameter::F_HOURLY or
                param->m_contentFrequency == Parameter::F_NO_FREQUENCY ) )
                return false;
        }
    }
    return ret;
}


void IcalInterpreter::makeAppointment( AppointmentBasics* &inAppBasics,
                                       AppointmentRecurrence* &inAppRecurrence,
                                       QList<AppointmentAlarm*> &inAppAlarmList )
{
    Appointment* t = new Appointment();
    t->m_appBasics = inAppBasics;
    t->m_appRecurrence = inAppRecurrence;
    t->m_appAlarms = inAppAlarmList;

    // make an event list
    if( inAppRecurrence )
    {
        qint64 seconds = t->m_appBasics->m_dtStart.secsTo( t->m_appBasics->m_dtEnd );
        QList<DateTime> list = t->m_appRecurrence->recurrenceStartDates( t->m_appBasics->m_dtStart );

        for( const DateTime dt : list )
        {
            Event e;
            e.m_uid = t->m_appBasics->m_uid;
            e.m_displayText = t->m_appBasics->m_summary;
            e.m_startDt = dt;
            QDateTime qdt = dt.addSecs( seconds );
            e.m_endDt = DateTime( qdt.date(), qdt.time(), qdt.timeZone(), e.m_startDt.isDate() );
            t->m_eventList.append( e );
            t->m_yearsInQuestion.insert( dt.date().year() );
        }
    }
    else
    {
        Event e;
        e.m_uid = t->m_appBasics->m_uid;
        e.m_displayText = t->m_appBasics->m_summary;
        e.m_startDt = t->m_appBasics->m_dtStart;
        e.m_endDt = t->m_appBasics->m_dtEnd;
        t->m_eventList.append( e );
        t->m_yearsInQuestion.insert( e.m_startDt.date().year() );
    }
    emit sigAppointmentReady( t );
}
