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
#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>
#include <QStringList>
#include <QVector>

#include "datetime.h"
#include "parameter.h"

struct Duration
{
    // init values
    void setZero();

    // set from values
    // @fixme: useful method?
    void setDuration( bool _minus, int _weeks, int _days, int _hours, int _minutes, int _seconds);

    // read from string, true on success
    bool readDuration( QString inDurationText );

    // value as seconds
    qint64 toSeconds() const;

    // just for debugging
    QString contentToString() const;

    // === Data ===
    bool    m_minus;
    int     m_weeks;
    int     m_days;
    int     m_hours;
    int     m_minutes;
    int     m_seconds;
};


struct Interval
{
    bool        m_hasDuration;
    DateTime    m_start;
    DateTime    m_end;
    Duration    m_duration;
};


struct Property
{
    // describes the way we store the data
    enum StorageType {
        PST_NOSTORAGE,  // example for recurrence rule, all content is inside of parameters
        PST_STRING,     // ... a QString
        PST_DATETIME,   // a single DateTime
        PST_ACTION,     // this is an action
        PST_INT,        // this parameter is an int
        PST_STRINGLIST, // ... a QStringList
        PST_DATETIMEVECTOR, // a vector of datetimes
        PST_DURATION,   // a duration
        PST_INTERVALVECTOR,   // a vector of interval structs, for RDATE
        PST_STATUSCODE, // Status Codes for PT_STATUS
        PST_TRANSPARENCY,   // for PT_TRANSP
        PST_DOUBLETUPLE     // GEO
    };

    /* The Properties given from the standard.
     * Because of C++, a name with "-" is written
     *  with underscore, example: REQUEST-STATUS ==> PT_REQUEST_STATUS
     */
    enum IcalPropertyType {
        PT_ACTION,          PT_ATTACH,          PT_ATTENDEE,
        PT_CALSCALE,        PT_CATEGORIES,      PT_CLASS,
        PT_COMMENT,         PT_COMPLETED,       PT_CONTACT,
        PT_CREATED,         PT_DESCRIPTION,     PT_DTEND,
        PT_DTSTAMP,         PT_DTSTART,         PT_DUE,
        PT_DURATION,        PT_EXDATE,
        PT_FREEBUSY,        PT_GEO,             PT_LAST_MODIFIED,
        PT_LOCATION,        PT_METHOD,          PT_ORGANIZER,
        PT_PERCENT_COMPLETE,PT_PRIORITY,        PT_PRODID,
        PT_RDATE,           PT_RECURRENCE_ID,   PT_RELATED_TO,
        PT_REPEAT,          PT_REQUEST_STATUS,  PT_RESOURCES,
        PT_RRULE,           PT_SEQUENCE,        PT_STATUS,
        PT_SUMMARY,         PT_TRANSP,          PT_TRIGGER,
        PT_TZID,            PT_TZNAME,          PT_TZOFFSETFROM,
        PT_TZOFFSETTO,      PT_TZURL,           PT_UID,
        PT_URL,             PT_VERSION,
        PT_PROPERTY_UNKNOWN        // if we really don't know
    };

    /* The action type of PT_ACTION.
     * Please note, that for action AUDIO, we do not support an
     *  attachment.
     */
    enum IcalActionType {
        A_AUDIO, A_DISPLY, A_EMAIL,
        A_OTHER     // in this case, we use the string to represent
    };

    /* Status Codes for PT_STATUS */
    enum IcalStatusCodeType {
        SCT_TENTATIVE, SCT_CONFIRMED, SCT_CANCELLED,        // VEvent
        SCT_NEEDS_ACTION, SCT_COMPLETED, SCT_IN_PROCESS,    // VToDo (+SCT_CANCELLED)
        SCT_DRAFT, SCT_FINAL                                // VJournal (+SCT_CANCELLED)
    };

    /* codes represent transparency of dates */
    enum IcalTransparencyType {
        TT_OPAQUE,      // Busy
        TT_TRANSPARENT  // Not busy
    };

    // === Methods ===

    // create a property
    Property();

    QString     contentToString() const;

    // just a single line like "DTSTART;TZID=America/New_York:19970610T090000"
    bool        readProperty( const QString inProp );

    // get a parameter by typename, return the property. true if found.
    bool        getParameterByType( const Parameter::IcalParameterType inSearchType, Parameter &outParam ) const;

    // Errors and Validation
    bool        m_hasErrors;   // true, if this property is not well formed
    bool        validate();

    /* convert from QString to IcalPropertyType and backward.
     * Note, that Strings are all UPPERCASE, especially for input strings
     */
    QString             propertyType( const IcalPropertyType inIpt ) const;
    IcalPropertyType    propertyType( const QString inIptString ) const;

    // Splits property parts
    static void         splitParts( const QString inToSplit, QString &outPropName,
                                    QString &outArgument, QStringList &outParameters );

    // === Data ===

    // holds the text
    QString m_thePropertyText;

    QString m_typeString;           // Property type as String
    IcalPropertyType m_type;        // Property type as enum

    // the way we store the parameter
    StorageType m_storageType;

    // Content Zone
    QString             m_content;              // Argument as String
    DateTime            m_contentDateTime;      // argument is a date or a datetime
    IcalActionType      m_contentAction;        // as an action
    int                 m_contentInteger;       // as int
    QStringList         m_contentStringList;    // as a list of strings
    QVector<DateTime>   m_contentDateTimeVector;// a vector of DateTimes
    Duration            m_contentDuration;      // as a duration
    QVector<Interval>   m_contentIntervalVector;// intervals, for RDATE
    IcalStatusCodeType  m_contentStatusCode;    // for PT_STATUS
    IcalTransparencyType m_contentTransparency; // for PT_TRANSP
    double              m_contentDoubleTuple[2]; // Tuple of float for GEO

    // List of parameters
    QVector<Parameter>    m_parameters; // attached parameters
};

#endif // PROPERTY_H
