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

#include "standarddaylightcomponent.h"


StandardDaylightComponent::StandardDaylightComponent()
{
}


QString StandardDaylightComponent::contentToString() const
{
    QString s( "{StdDay:" );
    for( const Property p : m_properties )
        s = s.append( p.contentToString() );
    return s.append( "}\n" );
}


void StandardDaylightComponent::readContentLine( const QString inContent )
{
    Property p = Property();
    if( p.readProperty( inContent ) )
        m_properties.append( p );
}


