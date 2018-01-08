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
#ifndef VTIMEZONECOMPONENT_H
#define VTIMEZONECOMPONENT_H

#include <QList>
#include <QString>
#include <QStringList>

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

    QList<Property*>                    m_properties;
    // parse parameters for Standard or Daylight
    ReadComponent                       m_activeComponent;
    QList<StandardDaylightComponent*>   m_StandardComponents;
    QList<StandardDaylightComponent*>   m_DaylightComponents;

};

#endif // VTIMEZONECOMPONENT_H
