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
#ifndef VTIMEZONECOMPONENT_H
#define VTIMEZONECOMPONENT_H

#include <QString>
#include <QStringList>
#include <QVector>

#include "property.h"
#include "standarddaylightcomponent.h"

/* VTimezone is defined in
 * rfc5545#section-3.6.5
 * It contains two sub-components: DAYLIGHT and STANDARD
 */
struct VTimezoneComponent
{
    enum ReadComponent {
        IN_VTIMEZONE,   // here
        IN_STANDARD,    // in STANDARD
        IN_DAYLIGHT     // in DAYLIGHT
    };

    // === Methods ===

    VTimezoneComponent();

    QString contentToString() const;

    void    readContentLine( const QString inContent );

    bool    validate();

    // === Data ===

    QVector<Property*>                  m_properties;
    // parse parameters for Standard or Daylight
    ReadComponent                       m_activeComponent;
    QVector<StandardDaylightComponent*> m_StandardComponents;
    QVector<StandardDaylightComponent*> m_DaylightComponents;

};

#endif // VTIMEZONECOMPONENT_H
