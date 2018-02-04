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
#ifndef ICALINTERPRETER_H
#define ICALINTERPRETER_H

#include "appointmentmanager.h"
#include "icalbody.h"
#include "property.h"
#include "datetime.h"

#include <utility>

#include <QVector>


class IcalInterpreter : public QObject
{
    Q_OBJECT

public:
    IcalInterpreter( QObject* parent = Q_NULLPTR  );
    void readIcal( const ICalBody &inIcal );

private:
    void readEvent( const VEventComponent inVEventComponent,
                    AppointmentBasics* &outAppBasics,
                    QVector<AppointmentAlarm*> &outAppAlarmVector,
                    AppointmentRecurrence* &outAppRecurrence );

    void readAlarm( const VAlarmComponent inVAlarmComponent,
                    const AppointmentBasics* inAppBasics,
                    AppointmentAlarm* &outAppAlarm );

    void readRecurrence( const Property inRecurrenceProperty,
                    AppointmentRecurrence* &outAppRecurrence );

    /* true, if inVEventComponent has NO RRULE,
     * true, if inVEventComponent has RRule with FREQ >= DAILY
     * else false.
     */
    bool eventHasUsableRRuleOrNone( const VEventComponent inVEventComponent );

    /* make an appointment and signal it to the outside world */
    void makeAppointment(AppointmentBasics* &inAppBasics,
                          AppointmentRecurrence* &inAppRecurrence,
                          QVector<AppointmentAlarm*>& inAppAlarmVector );

signals:
    void sigTickEvent( const int min, const int current, const int max );
    void sigTickVEvents( const int min, const int current, const int max );
    void sigAppointmentReady( Appointment* app );

public slots:
};

#endif // ICALINTERPRETER_H
