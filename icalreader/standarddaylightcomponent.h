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
#ifndef STANDARDDAYLIGHTCOMPONENT_H
#define STANDARDDAYLIGHTCOMPONENT_H

#include <QString>
#include <QStringList>
#include <QVector>

#include "property.h"

/* A sub-component of VTIMEZONE. This holds same sort of properties for
 * DAYLIGHT and STANDARD.
 * Definition see rfc5545#section-3.6.5
 */
struct StandardDaylightComponent
{
    // === Methods ===

    StandardDaylightComponent();

    QString contentToString() const;

    void    readContentLine( const QString inContent );

    // === Data ===

    QVector<Property*>  m_properties;
};

#endif // STANDARDDAYLIGHTCOMPONENT_H
