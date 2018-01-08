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
#ifndef ICALINTERPRETER_H
#define ICALINTERPRETER_H

#include "appointmentmanager.h"
#include "icalbody.h"
#include "property.h"
#include "datetime.h"


struct IcalInterpreter
{
    IcalInterpreter();

    void readIcal( const ICalBody &inIcal, AppointmentManager* &outAppManager );

    void readEvent( const VEventComponent* &inVEventComponent,
                    AppointmentBasics* &outAppBasics,
                    QList<AppointmentAlarm*> &outAppAlarmList,
                    AppointmentRecurrence* &outAppRecurrence );

    void readAlarm( const VAlarmComponent* inVAlarmComponent,
                    const AppointmentBasics* inAppBasics,
                    AppointmentAlarm* &outAppAlarm );

    void readRecurrence( const Property* inRecurrenceProperty,
                    AppointmentRecurrence* &outAppRecurrence );

    /* true, if inVEventComponent has NO RRULE,
     * true, if inVEventComponent has RRule with FREQ >= DAILY
     * else false.
     */
    bool eventHasUsableRRuleOrNone( const VEventComponent* inVEventComponent );

};

#endif // ICALINTERPRETER_H
