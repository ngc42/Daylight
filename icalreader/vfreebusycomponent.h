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
#ifndef VFREEBUSYCOMPONENT_H
#define VFREEBUSYCOMPONENT_H

#include <QList>
#include <QString>
#include <QStringList>

#include "property.h"

/* A VFREEBUSY component has no nested
 * components and is not nested in other components
 */

struct VFreeBusyComponent
{
    // === Methods ===

    VFreeBusyComponent();

    QString contentToString() const;

    void    readContentLine( const QString inContent );

    bool    validate();

    // === Data ===

    QList<Property*>    m_properties;
};

#endif // VFREEBUSYCOMPONENT_H
