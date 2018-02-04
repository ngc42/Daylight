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
#ifndef VJOURNALCOMPONENT_H
#define VJOURNALCOMPONENT_H

#include <QString>
#include <QStringList>
#include <QVector>

#include "property.h"

/* A VJOURNAL component has no nested
 *  components and is not nested in other components.
 * Can be RELATED-TO by VEVENT and VTODO*/
struct VJournalComponent
{
    // === Methods ===

    VJournalComponent();

    QString contentToString() const;

    void    readContentLine( const QString inContent );

    bool    validate();

    // === Data ===

    QVector<Property>    m_properties;
};

#endif // VJOURNALCOMPONENT_H
