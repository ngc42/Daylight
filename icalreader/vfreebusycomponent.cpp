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
#include "vfreebusycomponent.h"
#include <QDebug>


VFreeBusyComponent::VFreeBusyComponent()
{
}


QString VFreeBusyComponent::contentToString() const
{
    QString s( "{VFreeBusy:" );
    for( const Property* p : m_properties )
        s = s.append( p->contentToString() );
    return s.append( "}\n" );
}


void VFreeBusyComponent::readContentLine( const QString inContent )
{
    Property* p = new Property();
    if( p->readProperty( inContent ) )
        m_properties.append( p );
}


bool VFreeBusyComponent::validate()
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
    if( not ret ) qDebug() << " Validate VFreeBusyComponent FALSE";
    return ret;
}
