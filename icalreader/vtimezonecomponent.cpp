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
#include "vtimezonecomponent.h"
#include <QDebug>


VTimezoneComponent::VTimezoneComponent()
    :
      m_activeComponent( IN_VTIMEZONE )
{
}


QString VTimezoneComponent::contentToString() const
{
    QString s( "{VTimezone:" );
    for( const Property* p : m_properties )
        s = s.append( p->contentToString() );
    for( const StandardDaylightComponent* sc : m_StandardComponents )
        s = s.append( sc->contentToString() );
    for( const StandardDaylightComponent* dc : m_DaylightComponents )
        s = s.append( dc->contentToString() );
    return s.append( "}\n" );
}


void VTimezoneComponent::readContentLine( const QString inContent )
{
    if( inContent.startsWith( "BEGIN:STANDARD", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_STANDARD;
        StandardDaylightComponent* component = new StandardDaylightComponent();
        m_StandardComponents.append( component );
        return;
    }

    if( inContent.startsWith( "BEGIN:DAYLIGHT", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_DAYLIGHT;
        StandardDaylightComponent* component = new StandardDaylightComponent();
        m_DaylightComponents.append( component );
        return;
    }

    if( inContent.startsWith( "END:STANDARD", Qt::CaseInsensitive ) or
        inContent.startsWith( "END:DAYLIGHT", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_VTIMEZONE;
        return;
    }

    if( m_activeComponent == IN_VTIMEZONE )
    {
        Property *p = new Property();
        if( p->readProperty( inContent ) )
            m_properties.append( p );
        return;
    }

    if( m_activeComponent == IN_STANDARD )
    {
        m_StandardComponents.last()->readContentLine( inContent );
        return;
    }

    // m_activeComponent == IN_DAYLIGHT
    m_DaylightComponents.last()->readContentLine( inContent );
}


bool VTimezoneComponent::validate()
{
    int count_tzid = 0;     // MUST 1
    for( const Property* prop : m_properties )
    {
        if( prop->m_type == Property::PT_TZID )
            count_tzid++;
    }
    bool ret = ( ( m_StandardComponents.count() > 0 ) or ( m_DaylightComponents.count() > 0 ) )
            and ( count_tzid == 1 );
    if( not ret ) qDebug() << " Validate VTimeZoneComponent is FALSE";
    return ret;
}
