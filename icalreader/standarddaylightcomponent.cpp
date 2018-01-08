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
#include "standarddaylightcomponent.h"

StandardDaylightComponent::StandardDaylightComponent()
{
}


QString StandardDaylightComponent::contentToString() const
{
    QString s( "{StdDay:" );
    for( const Property* p : m_properties )
        s = s.append( p->contentToString() );
    return s.append( "}\n" );
}


void StandardDaylightComponent::readContentLine( const QString inContent )
{
    Property* p = new Property();
    if( p->readProperty( inContent ) )
        m_properties.append( p );
}


