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
#ifndef PARAMETER_H
#define PARAMETER_H

#include <QMultiMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTimeZone>

#include "datetime.h"

/* This class defindes property parameters as defined in
 * rfc5545#section-3.2
 * Example: The property "DESCRIPTION" has the parameter "ALTREP"
 * with content "cid:part1.0001@example.org".
 * DESCRIPTION;ALTREP="cid:part1.0001@example.org":The Fall'98 Wild
       Wizards Conference - - Las Vegas\, NV\, USA
 *
 * Parameters are created and stored by their properties.
*/
struct Parameter
{
    // describes the way we store the data
    enum StorageType {
        PST_BOOLEAN,    // this parameter is a boolean value
        PST_FREQUENCY,  // for RR_FREQ a const fom IcalFrequency
        PST_INT,        // this parameter is an int
        PST_INTSET,     // ... a QSet<int>
        PST_STRING,     // ... a QString
        PST_DATETIME,   // a single DateTime
        PST_STRINGLIST, // ... a QStringList
        PST_DAYMAP,     // special: QMultiMap for RR_BYDAY
        PST_DAY,        // special: IcalWeekDay for RR_WKST
        PST_CUTYPE,     // special for CUTYPE
        PST_PARTSTAT,   // special, stored in m_contentPartstat
        PST_TRIGREL,    // special, stored in m_contentPartstat
        PST_RELTYPEPARAM,   // special for RELTYPEPARAM
        PST_ROLEPARAM,  // special for ROLEPARAM
        PST_TIMEZONE,   // special for valid(!) TZ with TZIDPARAM
        PST_VALUES,     // special for VALUETYPEPARAM
    };

    /* Parameter Type (mostly) from the standard.
     *  For RRULE, we support some more types, this makes
     *  it easier to search for them.
     */
    enum IcalParameterType {
        ALTREPPARAM,    // ALTERNATE TEXT REPRESENTATION
        CNPARAM,        // COMMON NAME
        CUTYPEPARAM,    // CALENDAR USER TYPE
        DELFROMPARAM,   // DELEGATOR
        DELTOPARAM,     // DELEGATEE
        DIRPARAM,       // DIRECTORY ENTRY
        ENCODINGPARAM,  // INLINE ENCODING
        FMTTYPEPARAM,   // FORMAT TYPE
        FBTYPEPARAM,    // FREE/BUSY TIME TYPE
        LANGUAGEPARAM,  // LANGUAGE FOR TEXT
        MEMBERPARAM,    // GROUP OR LIST MEMBERSHIP
        PARTSTATPARAM,  // PARTICIPATION STATUS
        RANGEPARAM,     // RECURRENCE IDENTIFIER RANGE
        TRIGRELPARAM,   // ALARM TRIGGER RELATIONSHIP
        RELTYPEPARAM,   // RELATIONSHIP TYPE
        ROLEPARAM,      // PARTICIPATION ROLE
        RSVPPARAM,      // RSVP EXPECTATION
        SENTBYPARAM,    // SENT BY
        TZIDPARAM,      // REFERENCE TO TIME ZONE OBJECT
        VALUETYPEPARAM, // PROPERTY VALUE DATA TYPE
        OTHERPARAM,     // Other (iana or x-param) or unknown

        // From here, these are self-defines parameters
        // because RFC5545 does something special with RRULE
        // These are NOT part of the standard.
        RR_FREQ,
        RR_UNTIL,
        RR_COUNT,
        RR_INTERVAL,
        RR_BYSECOND,
        RR_BYMINUTE,
        RR_BYHOUR,
        RR_BYDAY,
        RR_BYMONTHDAY,
        RR_BYYEARDAY,
        RR_BYWEEKNO,
        RR_BYMONTH,
        RR_BYSETPOS,
        RR_WKST
    };

    /* Frequency type, used inside of RRULE */
    enum IcalFrequencyType {
        F_SECONDLY, F_MINUTELY, F_HOURLY,
        F_DAILY,    F_WEEKLY,   F_MONTHLY,
        F_YEARLY,
        F_NO_FREQUENCY  // this one is NOT part of the standard and shows an error
    };

    /* Enum for weekdays */
    enum class IcalWeekDayType {
        WD_MO = 1, WD_TU = 2, WD_WE = 3,
        WD_TH = 4, WD_FR = 5, WD_SA = 6,
        WD_SU = 7, WD_NO_DAY = 100
    };

    /* Enum for CUTYPE */
    enum IcalCutypeType {
        CT_INDIVIDUAL, CT_GROUP, CT_RESOURCE, CT_ROOM, CT_UNKNOWN,
        CT_OTHER        // x-name or iana token
    };

    /* Enum for PARTSTATPARAM */
    enum IcalPartstatParamType {
        PP_ACCEPTED,    PP_COMPLETED,       PP_DECLINED,    PP_DELEGATED,
        PP_IN_PROCESS,  PP_NEEDS_ACTION,    PP_TENTATIVE,
        PP_OTHER        // just if we don't know, not part of standard
    };

    /* Enum for TRIGRELPARAM */
    enum IcalTriggerRelParamType {
        TRP_START, TRP_END
    };

    /* Enum for RELTYPEPARAM */
    enum IcalReltypeParamType {
        RTP_CHILD, RTP_PARENT, RTP_SIBLING,
        RTP_OTHER        // x-name or iana token
    };

    /* Enum for ROLEPARAM */
    enum IcalRoleParamType {
        RP_CHAIR, RP_NON_PARTICIPANT, RP_OPT_PARTICIPANT, RP_REQ_PARTICIPANT,
        RP_OTHER        // something else like iana or whatever
    };

    /* Enum for VALUETYPEPARAM */
    enum IcalValueType {
        VT_BOOLEAN,     VT_CAL_ADDRESS, VT_DATE,    VT_DATE_TIME,
        VT_DURATION,    VT_FLOAT,       VT_INTEGER, VT_PERIOD,
        VT_RECUR,       VT_TEXT,        VT_TIME,    VT_URI,
        VT_UTC_OFFSET,
        VT_OTHER
    };

    // === Methods ===
    // create a paramter
    Parameter();

    QString     contentToString() const;             // debug output

    bool        readParameter( const QString s );  // Property wants us to read a parameter definition

    // Errors and Validation
    bool m_hasErrors;           // true, if we found an error during readParameter()
    bool        validate() const;      // can be used to validate. atm, returns !m_hasErrors

    // get type of this param
    IcalParameterType   type() const { return m_type; }
    QString             typeString() const;

    // remove the quotes arround string: "foo" -> foo
    void        stripQuotes( QString &inoutQuotedString );

    // === Data ===

    // the type of this parameter
    IcalParameterType m_type;

    // the way we store the parameter
    StorageType m_storageType;

    // Content Zone
    QString                     m_content;
    DateTime                    m_contentDateTime;
    int                         m_contentInteger;
    QSet<int>                   m_contentIntSet;
    QStringList                 m_contentStringList;
    bool                        m_contentBoolean;
    IcalFrequencyType           m_contentFrequency;
    QMultiMap<IcalWeekDayType, int> m_contentDayMap;
    IcalWeekDayType             m_contentWeekDay;
    IcalCutypeType              m_contentCutype;
    IcalPartstatParamType       m_contentPartstat;
    IcalTriggerRelParamType     m_contentTriggerRelParam;
    IcalReltypeParamType        m_contentReltypeParam;
    IcalRoleParamType           m_contentRoleParam;
    QTimeZone                   m_contentTimeZone;
    IcalValueType               m_contentValueType;
};

#endif // PARAMETER_H
