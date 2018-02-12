/*  This file is part of Daylight.
    Daylight - Calendarmanager, Appointment-program
    Copyright (C) 2014-2018  E.Lange

    Daylight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License.

    Daylight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QDebug>

#include "icalinterpreter.h"
#include "parameter.h"


IcalInterpreter::IcalInterpreter( QObject *parent )
    :
      QObject( parent )
{
}


void IcalInterpreter::readIcal( const ICalBody &inIcal )
{
    if( not inIcal.m_vEventComponents.isEmpty() )
    {
        int count = inIcal.m_vEventComponents.count();
        int num = 1;
        for( const VEventComponent component : inIcal.m_vEventComponents )
        {
            AppointmentBasics *basic = nullptr;
            QVector<AppointmentAlarm*> alarmList;
            AppointmentRecurrence *recurrence = nullptr;
            if( eventHasUsableRRuleOrNone( component ) ) // usable for our appointment structure?
            {
                readEvent( component, basic, alarmList, recurrence );
                makeAppointment( basic, recurrence, alarmList );
            }
            emit sigTickVEvents( 0, num++, count );
        }
    }
}


void IcalInterpreter::readEvent(const VEventComponent inVEventComponent,
                AppointmentBasics* &outAppBasics,
                QVector<AppointmentAlarm*>& outAppAlarmVector,
                AppointmentRecurrence* &outAppRecurrence )
{
    outAppBasics = new AppointmentBasics();
    bool haveRecurrence = false;
    bool have_rdate = false;    // rdate is somewhat special, see below...

    for( const Property p : inVEventComponent.m_properties )
    {
        if( p.m_type == Property::PT_DESCRIPTION )
        {
            outAppBasics->m_description = p.m_content;
            continue;
        }
        if( p.m_type == Property::PT_DTEND )
        {
            outAppBasics->m_dtEnd = p.m_contentDateTime;
            Parameter pp;
            if( p.getParameterByType( Parameter::TZIDPARAM, pp ) )
            {
                if( pp.m_contentTimeZone.isValid() )
                    outAppBasics->m_dtEnd.setTimeZone( pp.m_contentTimeZone );
            }
            continue;
        }
        if( p.m_type == Property::PT_DTSTART )
        {
            outAppBasics->m_dtStart = p.m_contentDateTime;
            Parameter pp;
            if( p.getParameterByType( Parameter::TZIDPARAM, pp ) )
            {
                if( pp.m_contentTimeZone.isValid() )
                    outAppBasics->m_dtStart.setTimeZone( pp.m_contentTimeZone );
            }
            continue;
        }
        if( p.m_type == Property::PT_DURATION )
        {
            outAppBasics->m_dtEnd = outAppBasics->m_dtStart.addSecs( p.m_contentDuration.toSeconds() );
            continue;
        }

        if( p.m_type == Property::PT_RDATE )
        {
            have_rdate = true;
            continue;
        }

        if( p.m_type == Property::PT_RRULE )
        {
            if( not haveRecurrence )
            {
                outAppRecurrence = new AppointmentRecurrence();
                haveRecurrence = true;
            }
            readRecurrenceRRule( p, outAppRecurrence );
            continue;
        }
        if( p.m_type == Property::PT_SUMMARY )
        {
            outAppBasics->m_summary = p.m_content;
            continue;
        }
        if( p.m_type == Property::PT_UID )
        {
            outAppBasics->m_uid = p.m_content;
            continue;
        }
        if( p.m_type == Property::PT_EXDATE )
        {
            if( not haveRecurrence )
            {
                outAppRecurrence = new AppointmentRecurrence();
                haveRecurrence = true;
            }
            if( p.m_storageType == Property::PST_DATETIME )
                outAppRecurrence->m_exceptionDates.append( p.m_contentDateTime );
            else // PST_DATETIMELIST
                outAppRecurrence->m_exceptionDates.append( p.m_contentDateTimeVector );
            continue;
        }
        if( p.m_type == Property::PT_SEQUENCE )
        {
            outAppBasics->m_sequence = p.m_contentInteger;
            continue;
        }
        if( p.m_type == Property::PT_TRANSP )
        {
            outAppBasics->m_busyFree =  p.m_contentTransparency == Property::TT_TRANSPARENT ?
                                AppointmentBasics::FREE : AppointmentBasics::BUSY;
            continue;
        }
    }

    if( have_rdate )
    {
        // The reason, why we loop again over all the properties is, that
        // the above finds out DT_START and DT_END. With this we can find out
        // length of an interval. Here, we can apply this interval length.
        quint64 intervalSeconds = outAppBasics->m_dtStart.secsTo( outAppBasics->m_dtEnd );
        for( const Property p : inVEventComponent.m_properties )
        {
            if( p.m_type != Property::PT_RDATE )
                continue;

            if( not haveRecurrence )
            {
                outAppRecurrence = new AppointmentRecurrence();
                haveRecurrence = true;
                // might get overwritten duing readRecurrenceRRule()
                outAppRecurrence->m_frequency = AppointmentRecurrence::RFT_FIXED_DATES;
            }
            readRecurrenceRDates( p, intervalSeconds, outAppBasics->m_dtStart, outAppRecurrence );
        }
    }

    for( const VAlarmComponent alarm : inVEventComponent.m_vAlarmComponents )
    {
        AppointmentAlarm *appAlarm = new AppointmentAlarm();
        readAlarm( alarm, outAppBasics, appAlarm );
        outAppAlarmVector.append( appAlarm );
    }
}


void IcalInterpreter::readAlarm( const VAlarmComponent inVAlarmComponent,
                                 const AppointmentBasics* inAppBasics,
                                 AppointmentAlarm* &outAppAlarm )
{
    for( const Property p : inVAlarmComponent.m_properties )
    {
        if( p.m_type == Property::PT_REPEAT )
        {
            outAppAlarm->m_repeatNumber = p.m_contentInteger;
            continue;
        }
        if( p.m_type == Property::PT_TRIGGER )
        {
            if( p.m_parameters.isEmpty() )
                outAppAlarm->m_alarmSecs = p.m_contentDuration.toSeconds();
            else
            {
                Parameter pp;
                if( p.getParameterByType( Parameter::TRIGRELPARAM, pp ) )
                {
                    outAppAlarm->m_alarmSecs = p.m_contentDuration.toSeconds();
                    if( pp.m_contentTriggerRelParam == Parameter::TRP_END )
                        outAppAlarm->m_alarmSecs +=
                                inAppBasics->m_dtStart.secsTo( (inAppBasics->m_dtEnd ) );
                    // No need to check for Parameter::TRP_START

                }
                else if( p.getParameterByType( Parameter::VALUETYPEPARAM, pp ) )
                {
                    if( pp.m_contentValueType == Parameter::VT_DATE_TIME )
                    {
                        DateTime dtAlarm;
                        dtAlarm = pp.m_contentDateTime;
                        outAppAlarm->m_alarmSecs = inAppBasics->m_dtStart.secsTo( dtAlarm );
                    }
                    if( pp.m_contentValueType == Parameter::VT_DURATION )
                    {
                        outAppAlarm->m_alarmSecs = p.m_contentDuration.toSeconds();
                    }
                }
            }
            continue;
        }
        if( p.m_type == Property::PT_DURATION )
        {
            if( p.m_storageType == Property::PST_DURATION )
                outAppAlarm->m_pauseSecs = p.m_contentDuration.toSeconds();
        }
    }
}


void IcalInterpreter::readRecurrenceRDates(const Property inRecurrenceProperty,
                const quint64 inIntervalSecondsToEndDate,
                const DateTime inStartDateTime,
                AppointmentRecurrence* &outAppRecurrence )
{
   for( Interval interval : inRecurrenceProperty.m_contentIntervalVector )
    {
        DateTime start;
        DateTime end;
        start = interval.m_start;
        if( interval.m_hasDuration )
            end = interval.m_start.addSecs( interval.m_duration.toSeconds() );
        else
            end = interval.m_end;

        Parameter timeZoneParam;
        if( inRecurrenceProperty.getParameterByType( Parameter::TZIDPARAM, timeZoneParam ) )
        {
            if( timeZoneParam.m_storageType == Parameter::PST_TIMEZONE )
                start.setTimeZone( timeZoneParam.m_contentTimeZone );
        }
        RecurringFixedIntervals fixedInterval;
        fixedInterval.setInterval( start, end );
        outAppRecurrence->m_recurFixedIntervals.append( fixedInterval );
    }

    for( DateTime startTime : inRecurrenceProperty.m_contentDateTimeVector )
    {
        DateTime newStartDate;
        if( startTime.isDate() )
        {
            newStartDate = QDateTime( startTime.date(), inStartDateTime.time() );
        }
        else
            newStartDate = startTime;

        DateTime endTime;
        endTime = newStartDate.addSecs( inIntervalSecondsToEndDate );
        Parameter timeZoneParam;
        if( inRecurrenceProperty.getParameterByType( Parameter::TZIDPARAM, timeZoneParam ) )
        {
            if( timeZoneParam.m_storageType == Parameter::PST_TIMEZONE )
                newStartDate.setTimeZone( timeZoneParam.m_contentTimeZone );
        }
        RecurringFixedIntervals fixedInterval;
        fixedInterval.setInterval( newStartDate, endTime );
        outAppRecurrence->m_recurFixedIntervals.append( fixedInterval );
    }
}


void IcalInterpreter::readRecurrenceRRule(const Property inRecurrenceProperty,
                                      AppointmentRecurrence* &outAppRecurrence  )
{
    int numberOfByRules = 0;
    // This is more or less just stupid translation between
    // Ical-consts and appointment const with just different names.
    // More, we limit shortest recurrence to daily.
    for( const Parameter p : inRecurrenceProperty.m_parameters )
    {
        if( p.m_type == Parameter::RR_FREQ )
        {
            switch( p.m_contentFrequency )
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
        else if( p.m_type == Parameter::RR_UNTIL )
        {
            outAppRecurrence->m_until = p.m_contentDateTime;
            outAppRecurrence->m_haveUntil = true;
        }
        else if( p.m_type == Parameter::RR_COUNT )
        {
            outAppRecurrence->m_count = p.m_contentInteger;
            outAppRecurrence->m_haveCount = true;
        }
        else if( p.m_type == Parameter::RR_INTERVAL )
        {
            outAppRecurrence->m_interval = p.m_contentInteger;
        }
        else if( p.m_type == Parameter::RR_BYDAY )
        {
            numberOfByRules++;
            for( const std::pair<Parameter::IcalWeekDayType, int> day : p.m_contentDaySet )
            {
                Parameter::IcalWeekDayType wd = day.first;
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
                outAppRecurrence->m_byDaySet.insert( std::make_pair( weekDay, day.second ) );
            }
        }
        else if( p.m_type == Parameter::RR_BYMONTHDAY )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMonthDaySet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYYEARDAY )
        {
            numberOfByRules++;
            outAppRecurrence->m_byYearDaySet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYWEEKNO )
        {
            numberOfByRules++;
            outAppRecurrence->m_byWeekNumberSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYMONTH )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMonthSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYHOUR )
        {
            numberOfByRules++;
            outAppRecurrence->m_byHourSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYMINUTE )
        {
            numberOfByRules++;
            outAppRecurrence->m_byMinuteSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYSECOND )
        {
            numberOfByRules++;
            outAppRecurrence->m_bySecondSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_BYSETPOS )
        {
            numberOfByRules++;
            outAppRecurrence->m_bySetPosSet = p.m_contentIntSet;
        }
        else if( p.m_type == Parameter::RR_WKST)
        {
            numberOfByRules++;
            switch( p.m_contentWeekDay )
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


bool IcalInterpreter::eventHasUsableRRuleOrNone( const VEventComponent inVEventComponent )
{
    bool ret = true;
    for( const Property p : inVEventComponent.m_properties )
    {
        if( p.m_type == Property::PT_RRULE )
        {
            Parameter param;
            bool found = p.getParameterByType( Parameter::RR_FREQ, param );
            if( found and ( param.m_contentFrequency == Parameter::F_SECONDLY or
                param.m_contentFrequency == Parameter::F_MINUTELY or
                param.m_contentFrequency == Parameter::F_HOURLY or
                param.m_contentFrequency == Parameter::F_NO_FREQUENCY ) )
                return false;
        }
    }
    return ret;
}


void IcalInterpreter::makeAppointment(AppointmentBasics* &inAppBasics,
                                       AppointmentRecurrence* &inAppRecurrence,
                                       QVector<AppointmentAlarm*> &inAppAlarmVector )
{
    Appointment* t = new Appointment();
    t->m_appBasics = inAppBasics;
    qDebug() << " IcalInterpreter::makeAppointment num alarm " << inAppAlarmVector.count();
    t->m_appRecurrence = inAppRecurrence;
    t->m_appAlarms = inAppAlarmVector;

    // fill up appointment
    t->m_minYear = 1000000;             // just a big number
    t->m_maxYear = 0;                   // just a small number
    t->m_userCalendarId = 0;            // here, we don't know
    t->m_uid = t->m_appBasics->m_uid;   // just forwarded
    t->m_haveAlarm = not inAppAlarmVector.isEmpty();

    if( inAppRecurrence )
        t->m_haveRecurrence = true;
    else
        t->m_haveRecurrence = false;

    connect( t, SIGNAL(sigTickEvent(int,int,int)), this, SIGNAL(sigTickEvent(int,int,int)) );
    t->makeEvents();    // make an event list
    disconnect( t, SIGNAL(sigTickEvent(int,int,int)), this, SIGNAL(sigTickEvent(int,int,int)) );
    emit sigAppointmentReady( t );
}


