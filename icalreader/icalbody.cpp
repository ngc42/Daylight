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

#include "icalbody.h"

#include <QDebug>


ICalBody::ICalBody()
    :
      m_activeComponent( IN_TOPLEVEL ),
      m_currentComponentName( "" )
{
}


void ICalBody::readContentLine( const QString inContent )
{
    // only toplevel components
    if( inContent.startsWith( "BEGIN:", Qt::CaseInsensitive ) and m_activeComponent == IN_TOPLEVEL )
    {
        QStringList parts = inContent.split( ':' );
        if( parts.count() == 2 )
        {
            QString componentName = parts.last();
            m_currentComponentName = componentName.trimmed().toUpper();
            if( m_currentComponentName.compare( "VEVENT" ) == 0 )
            {
                m_activeComponent = IN_VEVENT;
                VEventComponent component = VEventComponent();
                m_vEventComponents.append( component );
                return;
            }
            if( m_currentComponentName.compare( "VFREEBUSY" ) == 0 )
            {
                m_activeComponent = IN_VFREEBUSY;
                VFreeBusyComponent component = VFreeBusyComponent();
                m_vFreeBusyComponents.append( component );
                return;
            }
            if( m_currentComponentName.compare( "VJOURNAL" ) == 0 )
            {
                m_activeComponent = IN_VJOURNAL;
                VJournalComponent component = VJournalComponent();
                m_vJournalComponents.append( component );
                return;
            }
            if( m_currentComponentName.compare( "VTODO" ) == 0 )
            {
                m_activeComponent = IN_VTODO;
                VTodoComponent component = VTodoComponent();
                m_vToDoComponents.append( component );
                return;
            }
            if( m_currentComponentName.compare( "VTIMEZONE" ) == 0 )
            {
                m_activeComponent = IN_VTIMEZONE;
                VTimezoneComponent component = VTimezoneComponent();
                m_vTimezoneComponents.append( component );
                return;
            }

            m_activeComponent = IN_OTHER;
            return;
        }
        // ignore, if not equal to 2
    }

    if( inContent.startsWith( "END:", Qt::CaseInsensitive ) and
            ( m_activeComponent != IN_TOPLEVEL ) and
            ( m_currentComponentName.length() > 0 ) )
    {
        QStringList parts = inContent.split( ':' );
        if( parts.count() == 2 )
        {
            QString componentName = parts.last();
            componentName = componentName.trimmed().toUpper();
            if( m_currentComponentName.compare( componentName ) == 0 )
            {
                // yes, we expect to find it
                // finish reading component
                m_activeComponent = IN_TOPLEVEL;
                return;
            }
            // else: the current active component should care.
        }
    }

    if( inContent.startsWith( "X-", Qt::CaseInsensitive ) )
    {
        // ignore
        return;
    }

    switch( m_activeComponent )
    {
        case IN_TOPLEVEL:
        {
            Property prop = Property();
            prop.readProperty( inContent );
            m_properties.append( prop );
        }
        break;
        case IN_OTHER:
            // we should not read, what is not defined in the standard
            // or what we don't understand, like iana compononents
        break;
        case IN_VEVENT:
            m_vEventComponents.last().readContentLine( inContent );
        break;
        case IN_VFREEBUSY:
            m_vFreeBusyComponents.last().readContentLine( inContent );
        break;
        case IN_VJOURNAL:
            m_vJournalComponents.last().readContentLine( inContent );
        break;
        case IN_VTODO:
            m_vToDoComponents.last().readContentLine( inContent );
        break;
        case IN_VTIMEZONE:
            m_vTimezoneComponents.last().readContentLine( inContent );
        break;
    }
}


QString ICalBody::getContent() const
{
    QString s( "ICalBody: " );
    for( const Property p : m_properties )
        s = s.append( p.contentToString() );
    for( const VEventComponent vec : m_vEventComponents )
        s = s.append( vec.contentToString() );
    for( const VFreeBusyComponent vfbc : m_vFreeBusyComponents )
        s = s.append( vfbc.contentToString() );
    for( const VJournalComponent vjc : m_vJournalComponents )
        s = s.append( vjc.contentToString() );
    for( const VTodoComponent vtc : m_vToDoComponents )
        s = s.append( vtc.contentToString() );
    for( const VTimezoneComponent vtzc : m_vTimezoneComponents )
        s = s.append( vtzc.contentToString() );
    return s;
}


bool ICalBody::validateIcal()
{
    qDebug() << "BEGIN VALIDATE";
    bool ret = validateIcalBody();
    for( VEventComponent vec : m_vEventComponents )
        ret = ret and vec.validate();
    for( VFreeBusyComponent vfbc : m_vFreeBusyComponents )
        ret = ret and vfbc.validate();
    for( VJournalComponent vjc : m_vJournalComponents )
        ret = ret and vjc.validate();
    for( VTimezoneComponent vtzc : m_vTimezoneComponents )
        ret = ret and vtzc.validate();
    for( VTodoComponent vtc : m_vToDoComponents )
        ret = ret and vtc.validate();
    qDebug() << "END VALIDATE";
    return ret;
}


bool ICalBody::validateIcalBody()
{
    int count_version = 0;          // MUST: 1
    int count_calscale = 0;         // 0 or 1
    bool prod_id_modified = false;  // just a warning
    int count_required_prod_id = 0; // MUST: 1
    bool prop_ok = true;
    for( const Property prop : m_properties )
    {
        if( prop.m_hasErrors )
        {
            prop_ok = false;
            break;
        }
        if( prop.m_type == Property::PT_VERSION )
        {
            count_version++;
            continue;
        }
        if( prop.m_type == Property::PT_CALSCALE )
        {
            count_calscale++;
            continue;
        }
        if( prop.m_type == Property::PT_PRODID )
        {
            count_required_prod_id++;
            continue;
        }
    }

    if( count_version > 1 )
        qDebug() << " * Too many versions";
    if( prod_id_modified )
        qDebug() << " * ProdId modified - OK";
    if( count_required_prod_id == 0 )
    {
        Property p = Property();
        p.readProperty( "PRODID:-//DAYLIGHT//Modified//EN" );
        m_properties.append( p );
        count_required_prod_id++;
    }

    bool ret =  prop_ok and (count_version == 1 ) and (count_calscale < 2) and (count_required_prod_id == 1);
    if( not ret ) qDebug() << " Validate IcalBody FALSE";
    return ret;
}
