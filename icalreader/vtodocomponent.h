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
#ifndef VTODOCOMPONENT_H
#define VTODOCOMPONENT_H

#include <QList>
#include <QString>
#include <QStringList>

#include "property.h"
#include "valarmcomponent.h"

struct VTodoComponent
{
    enum ReadComponent {
        IN_VTODO,       // here
        IN_VALARM       // in component valarm
    };

    // === Methods ===

    VTodoComponent();

    QString contentToString() const;

    void    readContentLine( const QString inContent );

    /* validate()
     * @xixme: actually very useless,
     *  missing VAlarmComponents,
     * only required properties are checked
     */
    bool    validate();

    // === Data ===

    QList<Property*>        m_properties;
    // parse parameters for VTodo or VAlarm
    ReadComponent           m_activeComponent;
    QList<VAlarmComponent*> m_vAlarmComponents;
};

#endif // VTODOCOMPONENT_H
