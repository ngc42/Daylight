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

#include "parameter.h"

#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>


Parameter::Parameter()
    :
      m_hasErrors(false),
      m_type(OTHERPARAM),
      m_storageType(PST_STRING),
      m_content(""),
      m_contentInteger( 0 ),
      m_contentBoolean(false),
      m_contentFrequency( F_NO_FREQUENCY )
{
}


QString Parameter::contentToString() const
{
    QString ret = typeString();

    switch( m_storageType )
    {
        case PST_BOOLEAN:       ret += m_contentBoolean;
        break;
        case PST_FREQUENCY:
        {
            switch( m_contentFrequency )
            {
                case F_SECONDLY:    ret += "F_SECONDLY"; break;
                case F_MINUTELY:    ret += "F_MINUTELY"; break;
                case F_HOURLY:      ret += "F_HOURLY"; break;
                case F_DAILY:       ret += "F_DAILY"; break;
                case F_WEEKLY:      ret += "F_WEEKLY"; break;
                case F_MONTHLY:     ret += "F_MONTHLY"; break;
                case F_YEARLY:      ret += "F_YEARLY"; break;
                default:            ret += "UNKNOWN"; break;
            }
        }
        break;
        case PST_INT:           ret += QString( "%1" ).arg( m_contentInteger );    break;
        case PST_INTSET:
        {
            for( int i : m_contentIntSet )
                ret.append( QString( "%1," ).arg(i) );
        }
        break;
        case PST_STRING:        ret += m_content;   break;
        case PST_DATETIME:      ret += m_contentDateTime.toString(); break;
        case PST_STRINGLIST:
        {
            for( const QString s : m_contentStringList )
                ret += ret.append( s ).append( ',' );
        }
        break;
        case PST_DAYSET:
        {
            for( const std::pair<IcalWeekDayType, int> dayElem : m_contentDaySet )
            {
                QString s( "(%1:%2),");
                s = s.arg( static_cast<int>(dayElem.first) ).arg( dayElem.second );
                ret += s;
            }
        }
        break;
        case PST_DAY:
            ret = ret.append( QString( "%1").arg( static_cast<int>(m_contentWeekDay) ) );
        break;
        case PST_CUTYPE:        ret += static_cast<int>(m_contentCutype); break;
        case PST_PARTSTAT:      ret += static_cast<int>(m_contentPartstat); break;
        case PST_TRIGREL:       ret += static_cast<int>(m_contentTriggerRelParam); break;
        case PST_RELTYPEPARAM:  ret += static_cast<int>(m_contentReltypeParam); break;
        case PST_ROLEPARAM:     ret += static_cast<int>(m_contentRoleParam); break;
        case PST_TIMEZONE:      ret += QString( m_contentTimeZone.id() ); break;
        case PST_VALUES:        ret += static_cast<int>(m_contentValueType); break;
    }
    return ret;
}


bool Parameter::readParameter( const QString s )
{
    m_hasErrors = false;
    m_content = s;

    QStringList list = s.split( '=' );
    if( list.count() < 2 )
    {
        m_hasErrors = true;
        return false;
    }

    // All parameters have the form
    // paramName=argument, where argument might be
    //  a list sometimes.
    QString paramName = list.at(0);
    // join the rest.
    QString argument = list.at(1);
    for( int i = 2 ; i < list.count() ; i++ )
        argument.append( list.at(i) );

    if( paramName.isEmpty() or argument.isEmpty() )
    {
        m_hasErrors = true;
        return false;
    }

    // Example: ALTREP="CID:part3.msg.970415T083000@example.com"
    if( paramName.compare( "ALTREP", Qt::CaseInsensitive ) == 0 )
    {
        stripQuotes( argument );
        m_storageType = PST_STRING;
        m_content = argument;
        m_type = ALTREPPARAM;
        return true;
    }

    // Example: CN="John Smith"
    if( paramName.compare( "CN", Qt::CaseInsensitive ) == 0 )
    {
        stripQuotes( argument );
        m_storageType = PST_STRING;
        m_content = argument;
        m_type = CNPARAM;
        return true;
    }

    // Example: CUTYPE=GROUP
    if( paramName.compare( "CUTYPE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_CUTYPE;
        m_content = argument.toUpper();
        m_type = CUTYPEPARAM;
        if( m_content == "INDIVIDUAL" )     m_contentCutype = CT_INDIVIDUAL;
        else if( m_content == "GROUP" )     m_contentCutype = CT_GROUP;
        else if( m_content == "RESOURCE" )  m_contentCutype = CT_RESOURCE;
        else if( m_content == "ROOM" )      m_contentCutype = CT_ROOM;
        else if( m_content == "UNKNOWN" )   m_contentCutype = CT_UNKNOWN;
        else
        {
            m_storageType = PST_STRING;
            m_contentCutype = CT_OTHER;
        }
        return true;
    }

    // Example: DELEGATED-FROM="mailto:jsmith@example.com"
    // Argument might be a comma separated list
    if( paramName.compare( "DELEGATED-FROM", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        QStringList list = m_content.split( ',', QString::SkipEmptyParts );
        if( list.count() > 1 )
        {
            m_storageType = PST_STRINGLIST;
            m_contentStringList = list;
        }
        m_type = DELFROMPARAM;
        return true;
    }

    // Example: DELEGATED-TO="mailto:jdoe@example.com",
    //                       "mailto:jqpublic@example.com"
    // Argument might be a comma separated list
    if( paramName.compare( "DELEGATED-TO", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        QStringList list = m_content.split( ',', QString::SkipEmptyParts );
        if( list.count() > 1 )
        {
            m_storageType = PST_STRINGLIST;
            m_contentStringList = list;
        }
        m_type = DELTOPARAM;
        return true;
    }

    // Example: DIR="ldap://example.com:6666/o=ABC%20Industries,c=US"
    if( paramName.compare( "DIR", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        stripQuotes( argument );
        m_content = argument;
        m_type = DIRPARAM;
        return true;
    }

    // Example: ENCODING=BASE64
    // Please note, that we don't read attachments. So this parameter
    // might never occur.
    if( paramName.compare( "ENCODING", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument.toUpper();
        m_type = ENCODINGPARAM;
        return true;
    }

    // Example: FMTTYPE=text/plain
    // Please note, that we don't read attachments. So this parameter
    // might never occur.
    if( paramName.compare( "FMTTYPE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        m_type = FMTTYPEPARAM;
        return true;
    }

    // Example: FBTYPE=BUSY
    if( paramName.compare( "FBTYPE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument.toUpper();
        m_type = FBTYPEPARAM;
        return true;
    }

    // Example: LANGUAGE=en
    if( paramName.compare( "LANGUAGE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        m_type = LANGUAGEPARAM;
        return true;
    }

    // Example: MEMBER="mailto:aa@example.com","mailto:ab@example.com"
    // Argument might be a comma separated list
    if( paramName.compare( "MEMBER", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        QStringList list = m_content.split( ',', QString::SkipEmptyParts );
        if( list.count() > 1 )
        {
            m_storageType = PST_STRINGLIST;
            m_contentStringList = list;
        }
        m_type = MEMBERPARAM;
        return true;
    }

    // Example: PARTSTAT=DECLINED
    if( paramName.compare( "PARTSTAT", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_PARTSTAT;
        m_content = argument.toUpper();
        if( m_content == "ACCEPTED" )           m_contentPartstat = PP_ACCEPTED;
        else if( m_content == "COMPLETED" )     m_contentPartstat = PP_COMPLETED;
        else if( m_content == "DECLINED" )      m_contentPartstat = PP_DECLINED;
        else if( m_content == "DELEGATED" )     m_contentPartstat = PP_DELEGATED;
        else if( m_content == "IN-PROCESS" )    m_contentPartstat = PP_IN_PROCESS;
        else if( m_content == "NEEDS-ACTION" )  m_contentPartstat = PP_NEEDS_ACTION;
        else if( m_content == "TENTATIVE" )     m_contentPartstat = PP_TENTATIVE;
        else
        {
            m_contentPartstat = PP_OTHER;
            m_storageType = PST_STRING;
        }
        m_type = PARTSTATPARAM;
        return true;
    }

    // Example: RANGE=THISANDFUTURE
    if( paramName.compare( "RANGE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument.toUpper();
        if( m_content != "THISANDFUTURE" )
        {
            // THISANDFUTURE is the only allowed value
            m_hasErrors = true;
            return false;
        }
        m_type = RANGEPARAM;
        return true;
    }

    // Example: RELATED=END
    if( paramName.compare( "RELATED", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_TRIGREL;
        m_content = argument.toUpper();
        m_type = TRIGRELPARAM;

        if( m_content == "END" )        m_contentTriggerRelParam = TRP_END;
        else if( m_content == "START" ) m_contentTriggerRelParam = TRP_START;
        else
        {
            m_hasErrors = true;
            return false;
        }
        return true;
    }

    // Example: RELTYPE=SIBLING
    if( paramName.compare( "RELTYPE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_RELTYPEPARAM;
        m_content = argument.toUpper();
        m_type = RELTYPEPARAM;
        if( m_content == "PARENT" )         m_contentReltypeParam = RTP_PARENT;
        else if( m_content == "CHILD" )     m_contentReltypeParam = RTP_CHILD;
        else if( m_content == "SIBLING" )   m_contentReltypeParam = RTP_SIBLING;
        else
        {
            m_storageType = PST_STRING;
            m_contentReltypeParam = RTP_OTHER;
        }
        return true;
    }

    // Example: ROLE=CHAIR
    if( paramName.compare( "ROLE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_ROLEPARAM;
        m_content = argument.toUpper();
        m_type = ROLEPARAM;
        if( m_content == "CHAIR" )                  m_contentRoleParam = RP_CHAIR;
        else if( m_content == "NON-PARTICIPANT" )   m_contentRoleParam = RP_NON_PARTICIPANT;
        else if( m_content == "OPT-PARTICIPANT" )   m_contentRoleParam = RP_OPT_PARTICIPANT;
        else if( m_content == "REQ-PARTICIPANT" )   m_contentRoleParam = RP_REQ_PARTICIPANT;
        else
        {
            m_storageType = PST_STRING;
            m_contentRoleParam = RP_OTHER;
        }
        return true;
    }

    // Example: RSVP=TRUE
    if( paramName.compare( "RSVP", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument.toUpper();
        m_type = RSVPPARAM;
        bool valid = m_content == "TRUE" or m_content == "FALSE" ;
        if( valid )
        {
            m_contentBoolean = m_content == "TRUE";
            m_storageType = PST_BOOLEAN;
            return true;
        }
        m_hasErrors = true;
        return false;
    }

    // Example: SENT-BY="mailto:sray@example.com"
    // just a single address
    if( paramName.compare( "SENT-BY", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        stripQuotes( argument );
        m_content = argument;
        m_type = SENTBYPARAM;
        return true;
    }

    // Example: TZID=America/New_York
    if( paramName.compare( "TZID", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_STRING;
        m_content = argument;
        m_type = TZIDPARAM;
        QTimeZone tz( m_content.toUtf8() );
        if( tz.isValid() )
        {
            m_contentTimeZone = tz;
            m_storageType = PST_TIMEZONE;
        }
        // else: this could be a self defined TZ in VTimezoneComponent.
        //       so no reason to return an error here
        return true;
    }

    // Example: VALUE=BINARY
    if( paramName.compare( "VALUE", Qt::CaseInsensitive ) == 0 )
    {
        m_storageType = PST_VALUES;
        m_content = argument;
        m_type = VALUETYPEPARAM;
        if( m_content == "BOOLEAN" )            m_contentValueType = VT_BOOLEAN;
        else if( m_content == "CAL-ADDRESS" )   m_contentValueType = VT_CAL_ADDRESS;
        else if( m_content == "DATE" )          m_contentValueType = VT_DATE;
        else if( m_content == "DATE-TIME" )     m_contentValueType = VT_DATE_TIME;
        else if( m_content == "DURATION" )      m_contentValueType = VT_BOOLEAN;
        else if( m_content == "FLOAT" )         m_contentValueType = VT_DURATION;
        else if( m_content == "INTEGER" )       m_contentValueType = VT_INTEGER;
        else if( m_content == "PERIOD" )        m_contentValueType = VT_PERIOD;
        else if( m_content == "RECUR" )         m_contentValueType = VT_RECUR;
        else if( m_content == "TEXT" )          m_contentValueType = VT_TEXT;
        else if( m_content == "TIME" )          m_contentValueType = VT_TIME;
        else if( m_content == "URI" )           m_contentValueType = VT_URI;
        else if( m_content == "UTC-OFFSET" )    m_contentValueType = VT_UTC_OFFSET;
        else
        {
            m_storageType = PST_STRING;
            m_contentValueType = VT_OTHER;
        }
        return true;
    }

    // --------------------
    // for recurrence rule:

    if( paramName.compare( "FREQ", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument.toUpper();
        m_type = RR_FREQ;
        m_storageType = PST_FREQUENCY;
        if( m_content == "SECONDLY" )       m_contentFrequency = F_SECONDLY;
        else if( m_content == "MINUTELY" )  m_contentFrequency = F_MINUTELY;
        else if( m_content == "HOURLY" )    m_contentFrequency = F_HOURLY;
        else if( m_content == "DAILY" )     m_contentFrequency = F_DAILY;
        else if( m_content == "WEEKLY" )    m_contentFrequency = F_WEEKLY;
        else if( m_content == "MONTHLY" )   m_contentFrequency = F_MONTHLY;
        else if( m_content == "YEARLY" )    m_contentFrequency = F_YEARLY;
        else
        {
            m_storageType = PST_STRING;
            m_contentFrequency = F_NO_FREQUENCY;
            m_hasErrors = true;
            return false;
        }
        return true;
    }

    if( paramName.compare( "UNTIL", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument.toUpper();
        m_type = RR_UNTIL;

        DateTime dt;
        if( dt.readDateTime( m_content ) )
        {
            m_contentDateTime = dt;
            m_storageType = PST_DATETIME;
            return true;
        }
        return false;
    }

    if( paramName.compare( "COUNT", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_COUNT;
        bool ok = false;
        int value = m_content.toInt( &ok );
        if( ok and value > 0 )
        {
            m_storageType = PST_INT;
            m_contentInteger = value;
            return true;
        }
        m_hasErrors = true;
        return false;
    }

    if( paramName.compare( "INTERVAL", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_INTERVAL;
        bool ok = false;
        int value = m_content.toInt( &ok );
        if( ok and value > 0 )
        {
            m_storageType = PST_INT;
            m_contentInteger = value;
            return true;
        }
        m_hasErrors = true;
        return false;
    }

    if( paramName.compare( "BYSECOND", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYSECOND;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( value >= 0 ) and ( value <= 60 ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYMINUTE", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYMINUTE;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( value >= 0 ) and ( value < 60 ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYHOUR", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYHOUR;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( value >= 0 ) and ( value < 24 ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYDAY", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument.toUpper();
        m_type = RR_BYDAY;
        QStringList list = m_content.split( ',', QString::SkipEmptyParts, Qt::CaseInsensitive );
        QRegularExpression re( "([+-]?\\d*)(\\D*)" );
        IcalWeekDayType weekDay = IcalWeekDayType::WD_NO_DAY;
        for( const QString elem : list )
        {
            QRegularExpressionMatch match = re.match( elem );
            QString vString = match.captured( 1 );
            QString dString = match.captured( 2 );

            int value = 0;
            if( vString.size() > 0 )
            {
                // value is optional
                bool ok = true;
                value = vString.toInt( &ok );
                if( not ( ok and ( ( value > 0 and value < 54 ) or ( value < 0 and value > -54 ) ) ) )
                {
                    m_hasErrors = true;
                    return false;
                }
            }

            if( dString == "MO" )       weekDay = IcalWeekDayType::WD_MO;
            else if( dString == "TU" )  weekDay = IcalWeekDayType::WD_TU;
            else if( dString == "WE" )  weekDay = IcalWeekDayType::WD_WE;
            else if( dString == "TH" )  weekDay = IcalWeekDayType::WD_TH;
            else if( dString == "FR" )  weekDay = IcalWeekDayType::WD_FR;
            else if( dString == "SA" )  weekDay = IcalWeekDayType::WD_SA;
            else if( dString == "SU" )  weekDay = IcalWeekDayType::WD_SU;
            else
            {
                weekDay = IcalWeekDayType::WD_NO_DAY;
                m_hasErrors = true;
                return false;
            }

            std::pair<IcalWeekDayType,int> dayElem = std::make_pair(weekDay, value);
            m_contentDaySet.insert( dayElem );
        }
        m_storageType = PST_DAYSET;
        return true;
    }

    if( paramName.compare( "BYMONTHDAY", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYMONTHDAY;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( ( value > 0 and value < 32 ) or ( value < 0 and value > -32 ) ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYYEARDAY", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYYEARDAY;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( ( value > 0 and value <= 366 ) or ( value < 0 and value >= -366 ) ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYWEEKNO", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYWEEKNO;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( ( value > 0 and value <= 53 ) or ( value < 0 and value >= -53 ) ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYMONTH", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYMONTH;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and value > 0 and value <= 12 and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "BYSETPOS", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument;
        m_type = RR_BYSETPOS;
        bool ret = true;
        bool ok = false;
        int value;
        for( const QString s : m_content.split( ',', QString::SkipEmptyParts ) )
        {
            value = s.toInt( &ok );
            ret = ok and ( ( value > 0 and value <= 366 ) or ( value < 0 and value >= -366 ) ) and ( not m_contentIntSet.contains( value ) );
            if( not ret )
            {
                m_hasErrors = true;
                return false;
            }
            m_contentIntSet.insert( value );
        }
        m_storageType = PST_INTSET;
        return true;
    }

    if( paramName.compare( "WKST", Qt::CaseInsensitive ) == 0 )
    {
        m_content = argument.toUpper();
        m_type = RR_WKST;

        if( m_content == "MO" )       m_contentWeekDay = IcalWeekDayType::WD_MO;
        else if( m_content == "TU" )  m_contentWeekDay = IcalWeekDayType::WD_TU;
        else if( m_content == "WE" )  m_contentWeekDay = IcalWeekDayType::WD_WE;
        else if( m_content == "TH" )  m_contentWeekDay = IcalWeekDayType::WD_TH;
        else if( m_content == "FR" )  m_contentWeekDay = IcalWeekDayType::WD_FR;
        else if( m_content == "SA" )  m_contentWeekDay = IcalWeekDayType::WD_SA;
        else if( m_content == "SU" )  m_contentWeekDay = IcalWeekDayType::WD_SU;
        else
        {
            m_contentWeekDay = IcalWeekDayType::WD_NO_DAY;
            m_hasErrors = true;
            return false;
        }
        m_storageType = PST_DAY;
        return true;
    }

    // and now something completly different:
    m_type = OTHERPARAM;
    return true;
}


bool Parameter::validate() const
{
    if( m_hasErrors )
    {
        qDebug() << "ValidateParameter: Parameter has Errors " << m_content;
        return false;
    }
    // We didn't find a problem.
    return true;
}


QString Parameter::typeString() const
{
    if( m_type == ALTREPPARAM )     return "ALTREPPARAM:";
    if( m_type == CNPARAM )         return "CNPARAM:";
    if( m_type == CUTYPEPARAM )     return "CUTYPEPARAM:";
    if( m_type == DELFROMPARAM )    return "DELFROMPARAM:";
    if( m_type == DELTOPARAM )      return "DELTOPARAM:";
    if( m_type == DIRPARAM )        return "DIRPARAM:";
    if( m_type == ENCODINGPARAM )   return "ENCODINGPARAM:";
    if( m_type == FMTTYPEPARAM )    return "FMTTYPEPARAM:";
    if( m_type == FBTYPEPARAM )     return "FBTYPEPARAM:";
    if( m_type == LANGUAGEPARAM )   return "LANGUAGEPARAM:";
    if( m_type == MEMBERPARAM )     return "MEMBERPARAM:";
    if( m_type == PARTSTATPARAM )   return "PARTSTATPARAM:";
    if( m_type == RANGEPARAM )      return "RANGEPARAM:";
    if( m_type == TRIGRELPARAM )    return "TRIGRELPARAM:";
    if( m_type == RELTYPEPARAM )    return "RELTYPEPARAM:";
    if( m_type == ROLEPARAM )       return "ROLEPARAM:";
    if( m_type == RSVPPARAM )       return "RSVPPARAM:";
    if( m_type == SENTBYPARAM )     return "SENTBYPARAM:";
    if( m_type == TZIDPARAM )       return "TZIDPARAM:";
    if( m_type == VALUETYPEPARAM )  return "VALUETYPEPARAM:";

    // self defined for recurrence rule
    if( m_type == RR_FREQ )         return "RR_FREQ:";
    if( m_type == RR_UNTIL )        return "RR_UNTIL:";
    if( m_type == RR_COUNT )        return "RR_COUNT:";
    if( m_type == RR_INTERVAL )     return "RR_INTERVAL:";
    if( m_type == RR_BYSECOND )     return "RR_BYSECOND:";
    if( m_type == RR_BYMINUTE )     return "RR_BYMINUTE:";
    if( m_type == RR_BYHOUR )       return "RR_BYHOUR:";
    if( m_type == RR_BYDAY )        return "RR_BYDAY:";
    if( m_type == RR_BYMONTHDAY )   return "RR_BYMONTHDAY:";
    if( m_type == RR_BYYEARDAY )    return "RR_BYYEARDAY:";
    if( m_type == RR_BYWEEKNO )     return "RR_BYWEEKNO:";
    if( m_type == RR_BYMONTH )      return "RR_BYMONTH:";
    if( m_type == RR_BYSETPOS )     return "RR_BYSETPOS:";
    if( m_type == RR_WKST )         return "RR_WKST:";

    return "OTHERPARAM:";
}


void Parameter::stripQuotes( QString & inoutQuotedString )
{
    if( inoutQuotedString.startsWith( '\"' ) and inoutQuotedString.endsWith( '\"' ) )
    {
        inoutQuotedString = inoutQuotedString.mid( 1, inoutQuotedString.size() - 2 );
    }
}
