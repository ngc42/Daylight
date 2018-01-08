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
#include "valarmcomponent.h"
#include <QDebug>


VAlarmComponent::VAlarmComponent()
{
}


QString VAlarmComponent::contentToString() const
{
    QString s("{Alarm:");
    for( const Property *p : m_properties )
        s = s.append( p->contentToString() );
    return s.append( "}\n" );
}


void VAlarmComponent::readContentLine( const QString inContent )
{
    // there is no "BEGIN" or "END" in content,
    // so just push parameters
    Property *p = new Property();
    if( p->readProperty( inContent ) )
        m_properties.append( p );
}


bool VAlarmComponent::validate()
{
    int count_action = 0;       // MUST 1
    int count_trigger = 0;      // Must 1
    for( Property* prop : m_properties )
    {
        if( not prop->validate() )
            return false;
        if( prop->m_type == Property::PT_ACTION )
            count_action++;
        else if( prop->m_type == Property::PT_TRIGGER )
            count_trigger++;
    }
    qDebug() << "  Validate " << count_action << count_trigger ;
    bool ret = ( count_action == 1 ) and ( count_trigger == 1 );
    if( not ret ) qDebug() << " Validate VAlarmComponent FALSE";
    return ret;
}
