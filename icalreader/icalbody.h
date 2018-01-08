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
#ifndef ICALBODY_H
#define ICALBODY_H

#include <QList>
#include <QString>
#include <QStringList>

#include "property.h"

#include "veventcomponent.h"
#include "vfreebusycomponent.h"
#include "vjournalcomponent.h"
#include "vtodocomponent.h"
#include "vtimezonecomponent.h"

struct ICalBody
{
    enum ReadComponent {
        IN_TOPLEVEL,        // in this body/header
        IN_OTHER,           // x-component / iana-component
        IN_VEVENT,          // VEventComponent
        IN_VFREEBUSY,
        IN_VJOURNAL,
        IN_VTODO,
        IN_VTIMEZONE
    };

    ICalBody();
    void                readContentLine( const QString inContent );
    QString             getContent() const;

    /* validateIcal()
     * validates all the data. Can be called, after everything is read
     *  and stored in component data. Eventually changes data to ensure,
     *  everythings works fine.
     * Returns  true - everything was ok
     */
    bool validateIcal();
    bool validateIcalBody();



    QList<Property*>            m_properties;
    ReadComponent               m_activeComponent;
    QString                     m_currentComponentName;
    QList<VEventComponent*>     m_vEventComponents;
    QList<VFreeBusyComponent*>  m_vFreeBusyComponents;
    QList<VJournalComponent*>   m_vJournalComponents;
    QList<VTodoComponent*>      m_vToDoComponents;
    QList<VTimezoneComponent*>  m_vTimezoneComponents;
};

#endif // ICALBODY_H
