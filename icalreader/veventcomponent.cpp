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

#include <QDateTime>
#include <QDebug>

#include "veventcomponent.h"


VEventComponent::VEventComponent()
    :
      m_activeComponent( IN_VEVENT )
{
}


QString VEventComponent::contentToString() const
{
    QString s( "{VEVENT:" );
    for( const Property p : m_properties )
        s = s.append( p.contentToString() );
    for( const VAlarmComponent c : m_vAlarmComponents )
        s = s.append( c.contentToString() );
    return s.append( "}\n" );
}


void VEventComponent::readContentLine( const QString inContent )
{
    if( inContent.startsWith( "BEGIN:VALARM", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_VALARM;
        VAlarmComponent component = VAlarmComponent();
        m_vAlarmComponents.append( component );
        return;
    }
    if( inContent.startsWith( "END:VALARM", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_VEVENT;
        return;
    }

    if( m_activeComponent == IN_VEVENT )
    {
        Property p = Property();
        if( p.readProperty( inContent ) )
        {
            // Put DT-START first, this makes it easier to test for DT_END or DURATION
            if( p.m_type == Property::PT_DTSTART )
                m_properties.prepend( p );
            else
                m_properties.append( p );
        }
    }
    else
        m_vAlarmComponents.last().readContentLine( inContent );
}


bool VEventComponent::validate()
{
    int count_uid = 0;          // MUST 1
    int count_dtstamp = 0;      // Must 1
    int count_dtstart = 0;      // MUST 1
    int count_dtend_duration = 0;   // < 2
    for( Property &prop : m_properties )
    {
        if( not prop.validate() )
            return false;
        if( prop.m_type == Property::PT_UID )
        {
            count_uid++;
            continue;
        }
        if( prop.m_type == Property::PT_DTSTAMP )
        {
            count_dtstamp++;
            continue;
        }
        if( prop.m_type == Property::PT_DTSTART )
        {
            count_dtstart++;
            continue;
        }
        if( prop.m_type == Property::PT_DTEND or prop.m_type == Property::PT_DURATION )
        {
            count_dtend_duration++;
            continue;
        }
        if( prop.m_type == Property::PT_RRULE )
        {
            int count_and_until = 0;        // MUST 0 or 1
            for( const Parameter param : prop.m_parameters )
            {
                if( param.m_type == Parameter::RR_COUNT or param.m_type == Parameter::RR_UNTIL )
                    count_and_until++;
            }
            if( count_and_until > 1 )
            {
                qDebug() << "VEventComponent::validate(): too many COUNT or UNTIL in RRULE";
                return false;
            }
        }
    }
    if( count_uid == 0 )
    {
        Property p = Property();
        QString u( "UID:DAYLIGHT-Modified-Uid" );
        u = u.append( QDateTime::currentDateTime().toString( "yyyyMMddhhmmss" ) );
        if( p.readProperty( u ) )
        {
            m_properties.append( p );
            count_uid++;
            qDebug() << " * Append UID" << u ;
        }
    }
    if( count_dtstamp == 0 )
    {
        Property p = Property();
        QString s( "DTSTAMP:" );
        s = s.append( QDateTime::currentDateTime().toString( "yyyyMMddThhmmssZ" ) );
        if( p.readProperty( s ) )
        {
            m_properties.append( p );
            count_dtstamp++;
            qDebug() << " * Append DTSTAMP" << s ;
        }
    }

    bool alarm_ok = true;
    for( VAlarmComponent va : m_vAlarmComponents )
        alarm_ok = alarm_ok and va.validate();
    if( not alarm_ok )
        qDebug() << " * Alarm not ok.";

    bool ret = alarm_ok and ( count_uid == 1 ) and ( count_dtstamp == 1 ) and
            ( count_dtstart == 1 ) and ( count_dtend_duration < 2 ) ;
    if( not ret ) qDebug() << " Validate VEventComponent FALSE";
    return ret;
}
