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
#include "vtodocomponent.h"
#include <QDateTime>
#include <QDebug>

VTodoComponent::VTodoComponent()
    :
      m_activeComponent( IN_VTODO )
{
}


QString VTodoComponent::contentToString() const
{
    QString s( "{VTODO:" );
    for( const Property* p : m_properties )
        s = s.append( p->contentToString() );
    for( const VAlarmComponent* c : m_vAlarmComponents )
        s = s.append( c->contentToString() );
    return s.append( "}\n" );
}


void VTodoComponent::readContentLine( const QString inContent )
{
    if( inContent.startsWith( "BEGIN:VALARM", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_VALARM;
        VAlarmComponent *component = new VAlarmComponent();
        m_vAlarmComponents.append( component );
        return;
    }
    if( inContent.startsWith( "END:VALARM", Qt::CaseInsensitive ) )
    {
        m_activeComponent = IN_VTODO;
        return;
    }

    if( m_activeComponent == IN_VTODO )
    {
        Property *p = new Property();
        if( p->readProperty( inContent ) )
            m_properties.append( p );
    }
    else
        m_vAlarmComponents.last()->readContentLine( inContent );
}


bool VTodoComponent::validate()
{
    int count_uid = 0;          // MUST exact 1
    int count_dtstamp = 0;      // Must exact 1
    for( Property* &prop : m_properties )
    {
        if( prop->m_type == Property::PT_UID )
            count_uid++;
        else if( prop->m_type == Property::PT_DTSTAMP )
            count_dtstamp++;
    }
    bool ret = ( count_uid == 1 ) and ( count_dtstamp == 1 );
    if( not ret ) qDebug() << " Validate ToDo FALSE";
    return ret;
}
