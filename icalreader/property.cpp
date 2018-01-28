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
#include "property.h"

#include <QRegularExpression>
#include <QDebug>

Property::Property()
    :
      m_hasErrors( false ),
      m_thePropertyText(""),
      m_typeString(""),
      m_type(PT_PROPERTY_UNKNOWN),
      m_storageType( Property::PST_NOSTORAGE ),
      m_content(""),
      m_contentInteger(0)
{
}


QString Property::contentToString() const
{
    QString s(" (Property:[");
    s = s.append( propertyType( m_type ) ).append( "]:[" );

    switch( m_storageType )
    {
        case PST_NOSTORAGE:
        break;
        case PST_STRING:    s = s.append( m_content );
        break;
        case PST_DATETIME:  s = s.append( m_contentDateTime.toString() );
        break;
        case PST_ACTION:    s = s.append( QString( "%1" ).arg( static_cast<int>(m_contentAction) ) );
        break;
        case PST_INT:       s = s.append( QString( "%1" ).arg( m_contentInteger ) );
        break;
        case PST_STRINGLIST:
        {
            for(const QString v : m_contentStringList )
                s = s.append( v ).append( ',' );
        }
        break;
        case PST_DATETIMELIST:
        {
            for( const DateTime dt : m_contentDateTimeList )
                s = s.append( dt.toString() ).append( ',' );
        }
        break;
        case PST_DURATION:
        {
            QString a;
            a = a.append( "%1%2W%3DT%4H%5M%6S")
                .arg( m_contentDuration.minus ? '-' : '+')
                .arg( m_contentDuration.weeks )
                .arg( m_contentDuration.days )
                .arg( m_contentDuration.hours )
                .arg( m_contentDuration.minutes )
                .arg( m_contentDuration.seconds );
            s = s.append( a );
        }
        break;
        case PST_STATUSCODE:
            s = s.append( QString( "%1" ).arg( static_cast<int>(m_contentStatusCode) ) );
        break;
        case PST_TRANSPARENCY:
            s = s.append( QString( "%1" ).arg( m_contentTransparency == TT_OPAQUE ? "Opaque" : "Transparent" ) );
        break;
        case PST_DOUBLETUPLE:
            s = s.append( QString( "%1 / %2" )
                          .arg( m_contentDoubleTuple[0], 5, 'f' ).arg( m_contentDoubleTuple[1], 5, 'f' ) );
        break;
    }
    s = s.append( "]" );

    if( m_parameters.isEmpty() )
        return s.append( "[no param])\n");
    for( Parameter* p : m_parameters )
    {
        QString description( "{" );
        description = description.append( p->contentToString() ).append("}");
        s = s.append( description );
    }
    return s.append( ")\n" );
}


bool Property::readProperty( const QString inProp )
{
    m_hasErrors = false;
    m_thePropertyText = inProp;

    QString propertyArgument;
    QStringList parameterList;
    Property::splitParts( inProp, m_typeString, propertyArgument, parameterList );
    m_type = propertyType( m_typeString );

    // fill the parameters
    for( QString ps : parameterList )
    {
        Parameter *p = new Parameter();
        if( not p->readParameter( ps ) )
        {
            m_hasErrors = true;
            return false;
        }
        m_parameters.append( p );
    }

    // now, that we have our parameters, store the
    //  content of this property.
    if( m_type == Property::PT_RRULE )
    {
        m_storageType = Property::PST_NOSTORAGE;
        return true;
    }

    if( propertyArgument.isEmpty() )
    {
        m_hasErrors = true;
        return false;
    }

    if( m_type == Property::PT_ACTION )
    {
        m_content = propertyArgument.toUpper();
        if( propertyArgument == "AUDIO" )
            m_contentAction = A_AUDIO;
        else if( propertyArgument == "DISPLAY" )
            m_contentAction = A_DISPLY;
        else if( propertyArgument == "EMAIL" )
            m_contentAction = A_EMAIL;
        else
            m_contentAction = A_OTHER;
        m_storageType = Property::PST_ACTION;
        return true;
    }

    if( m_type == Property::PT_ATTACH )
    {
        // sorry, we do not store anything here
        m_content.clear();
        m_storageType = Property::PST_NOSTORAGE;
        return true;
    }

    if( m_type == Property::PT_CALSCALE )
    {
        m_content = propertyArgument.toUpper();
        m_storageType = Property::PST_STRING;
        if( m_content != "GREGORIAN" )
        {
            m_hasErrors = true;
            return false;
        }
        return true;
    }

    if( m_type == Property::PT_COMPLETED or m_type == Property::PT_CREATED or
        m_type == Property::PT_DTEND or m_type == Property::PT_DTSTAMP or
        m_type == Property::PT_DTSTART or m_type == Property::PT_DUE or
        m_type == Property::PT_LAST_MODIFIED )
    {
        m_content = propertyArgument.toUpper();
        DateTime dt;
        if( dt.readDateTime( m_content ) )
        {
            m_contentDateTime = dt;
            m_storageType = Property::PST_DATETIME;
            return true;
        }
        return false;
    }

    if( m_type == PT_EXDATE )
    {
        m_content = propertyArgument.toUpper();
        if( propertyArgument.contains( ',' ) )
        {
            // a list
            QStringList list = propertyArgument.split( ',', QString::SkipEmptyParts );
            for( QString s : list )
            {
                DateTime dt;
                if( dt.readDateTime( s ) )
                {
                    qDebug() << " read DT-List : " << s << " res= " << dt.toString();
                    m_contentDateTimeList.append( dt );
                }
                else
                    return false;
            }
            m_storageType = Property::PST_DATETIMELIST;
            return true;
        }
        DateTime dt;
        if( dt.readDateTime( m_content ) )
        {
            qDebug() << " read single DT : " << m_content << " res= " << dt.toString();
            m_contentDateTime = dt;
            m_storageType = Property::PST_DATETIME;
            return true;
        }
        return false;
    }

    if( m_type == PT_CATEGORIES or m_type == PT_FREEBUSY or
        m_type == PT_RESOURCES )
    {
        if( propertyArgument.contains( ',' ) )
        {
            // a list
            QStringList list = propertyArgument.split( ',', QString::SkipEmptyParts );
            for( QString s : list )
            {
                m_contentStringList.append( s.trimmed() );
            }
            m_storageType = Property::PST_STRINGLIST;
            return true;
        }
        m_storageType = Property::PST_STRING;
        m_content = propertyArgument;
        return true;
    }

    if( m_type == PT_DURATION or m_type == PT_TRIGGER )
    {
        m_content = propertyArgument.toUpper();
        int index_P = m_content.indexOf( 'P' );
        if( m_type == PT_TRIGGER and index_P < 0 )
        {
            // trigger can also have a datetime :-/
            m_storageType = Property::PST_STRING;
            return true;
        }

        int index_M = m_content.indexOf( '-' );
        if( index_P < 0 )
        {
            m_hasErrors = true;
            return false;
        }
        m_storageType = PST_DURATION;
        m_contentDuration.minus = index_M > -1;
        m_contentDuration.weeks = 0;
        m_contentDuration.days = 0;
        m_contentDuration.hours = 0;
        m_contentDuration.minutes = 0;
        m_contentDuration.seconds = 0;
        QString tmp( m_content );
        tmp = tmp.right( tmp.count() - index_P - 1 );

        // test
        QStringList list = propertyArgument.split( 'T', QString::SkipEmptyParts );
        if( list.count() == 0 )
        {

            m_hasErrors = true;
            return false;
        }
        if( list.at( 0 ).contains( 'W' ) )
        {
            QRegularExpression re( "(\\d*)W" );
            QRegularExpressionMatch match = re.match( list.at(0) );
            QString vString = match.captured( 1 );
            bool ok = false;
            m_contentDuration.weeks = vString.toInt( &ok );
            if( not (ok and m_contentDuration.weeks > 0 ) )
            {
                m_hasErrors = true;
                return false;
            }
            // According to standard, "dur-week" has no follow-ups
            return true;
        }
        if( list.at( 0 ).contains( 'D' ) )
        {
            QRegularExpression re( "(\\d*)D" );
            QRegularExpressionMatch match = re.match( list.at(0) );
            QString vString = match.captured( 1 );
            bool ok = false;
            m_contentDuration.days = vString.toInt( &ok );
            if( not (ok and m_contentDuration.days >= 0 ) )
            {
                m_hasErrors = true;
                return false;
            }
        }
        if( list.count() == 2 ) // time follow-up?
        {
            if( list.at( 1 ).contains( 'H' ) )
            {
                QRegularExpression re( "(\\d*)H" );
                QRegularExpressionMatch match = re.match( list.at(1) );
                QString vString = match.captured( 1 );
                bool ok = false;
                m_contentDuration.hours = vString.toInt( &ok );
                if( not (ok and m_contentDuration.hours >= 0 ) )
                {
                    m_hasErrors = true;
                    return false;
                }
            }
            if( list.at( 1 ).contains( 'M' ) )
            {
                QRegularExpression re( "(\\d*)M" );
                QRegularExpressionMatch match = re.match( list.at(1) );
                QString vString = match.captured( 1 );
                bool ok = false;
                m_contentDuration.minutes = vString.toInt( &ok );
                if( not (ok and m_contentDuration.minutes >= 0 ) )
                {
                    m_hasErrors = true;
                    return false;
                }
            }
            if( list.at( 1 ).contains( 'S' ) )
            {
                QRegularExpression re( "(\\d*)S" );
                QRegularExpressionMatch match = re.match( list.at(1) );
                QString vString = match.captured( 1 );
                bool ok = false;
                m_contentDuration.seconds = vString.toInt( &ok );
                if( not (ok and m_contentDuration.seconds >= 0 ) )
                {
                    m_hasErrors = true;
                    return false;
                }
            }
        }
        return true;
    }

    if( m_type == PT_GEO )
    {
        m_content = propertyArgument;
        QStringList list = m_content.split( ';', QString::SkipEmptyParts );
        if( list.count() == 2 )
        {
            bool ok = true;
            // longitude and lattitude
            QString list0 = list.at(0);
            double x = list0.toDouble( &ok );
            if( not ok ) {m_hasErrors = true; return false;}

            QString list1 = list.at(1);
            double y = list1.toDouble( &ok );
            if( not ok ) {m_hasErrors = true; return false;}

            m_contentDoubleTuple[0] = x;
            m_contentDoubleTuple[1] = y;
            m_storageType = PST_DOUBLETUPLE;
            return true;
        }
        else
        {
            m_hasErrors = true;
            return false;
        }
    }

    if( m_type == PT_PERCENT_COMPLETE or m_type == PT_PRIORITY or
        m_type == PT_REPEAT or m_type == PT_SEQUENCE )
    {
        m_content = propertyArgument;
        bool ok = false;
        m_contentInteger = propertyArgument.toInt( &ok );
        if( not ( ok and m_contentInteger >= 0 ) )
        {
            m_hasErrors = true;
            return false;
        }
        m_storageType = PST_INT;
        return true;
    }

    if( m_type == PT_STATUS )
    {
        m_content = propertyArgument.toUpper();
        if( m_content == "TENTATIVE" )          m_contentStatusCode = SCT_TENTATIVE;
        else if( m_content == "CONFIRMED" )     m_contentStatusCode = SCT_CONFIRMED;
        else if( m_content == "CANCELLED" )     m_contentStatusCode = SCT_CANCELLED;
        else if( m_content == "NEEDS-ACTION" )  m_contentStatusCode = SCT_NEEDS_ACTION;
        else if( m_content == "COMPLETED" )     m_contentStatusCode = SCT_COMPLETED;
        else if( m_content == "IN-PROCESS" )    m_contentStatusCode = SCT_IN_PROCESS;
        else if( m_content == "DRAFT" )         m_contentStatusCode = SCT_DRAFT;
        else if( m_content == "FINAL" )         m_contentStatusCode = SCT_FINAL;
        else
        {
            m_hasErrors = true;
            return false;
        }
        m_storageType = PST_STATUSCODE;
        return true;
    }

    if( m_type == PT_TRANSP )
    {
        m_content = propertyArgument.toUpper();
        if( m_content == "OPAQUE" )             m_contentTransparency = TT_OPAQUE;
        else if( m_content == "TRANSPARENT" )   m_contentTransparency = TT_TRANSPARENT;
        else
        {
            m_hasErrors = true;
            return false;
        }
        m_storageType = PST_TRANSPARENCY;
        return true;
    }

    if( m_type == PT_VERSION )
    {
        m_content = propertyArgument;
        if( propertyArgument != "2.0" )
        {
            m_hasErrors = true;
            return false;
        }
        m_storageType = Property::PST_STRING;
        return true;
    }

    // everything else is just storen in a string
    m_storageType = Property::PST_STRING;
    m_content = propertyArgument;    // always store a string
    return true;
}


bool Property::getParameterByType( const Parameter::IcalParameterType inSearchType,
                                   Parameter* &outParam ) const
{
    if( m_parameters.empty() ) return false;
    for( Parameter* param : m_parameters )
        if( param->m_type == inSearchType )
        {
            outParam = param;
            return true;
        }
    return false;
}


qint64 Property::durationToSeconds() const
{
    qint64 secs = m_contentDuration.weeks * 7 * 24 * 60 * 60;
    secs += m_contentDuration.days * 24 * 60 * 60;
    secs += m_contentDuration.hours * 60 * 60;
    secs += m_contentDuration.minutes * 60;
    secs += m_contentDuration.seconds;
    return m_contentDuration.minus ? -secs : secs;
}


bool Property::validate()
{
    if( m_hasErrors )
    {
        qDebug() << "Validate: Property has Errors " << m_thePropertyText;
        return false;
    }
    if( m_type == PT_PROPERTY_UNKNOWN )
    {
        qDebug() << "Validate: Unknown property " << m_thePropertyText;
        return false;
    }
    if( m_type == PT_RRULE )
    {
        bool ret = true;
        for( const Parameter* param : m_parameters )
            ret = ret and param->validate();
        return ret;
    }
    return true;
}


QString Property::propertyType( const IcalPropertyType inIpt ) const
{
    if( inIpt == PT_ACTION )            return QString( "ACTION" );
    if( inIpt == PT_ATTACH )            return QString( "ATTACH" );
    if( inIpt == PT_ATTENDEE )          return QString( "ATTENDEE" );
    if( inIpt == PT_CALSCALE )          return QString( "CALSCALE" );
    if( inIpt == PT_CATEGORIES )        return QString( "CATEGORIES" );
    if( inIpt == PT_CLASS )             return QString( "CLASS" );
    if( inIpt == PT_COMMENT )           return QString( "COMMENT" );
    if( inIpt == PT_COMPLETED )         return QString( "COMPLETED" );
    if( inIpt == PT_CONTACT )           return QString( "CONTACT" );
    if( inIpt == PT_CREATED )           return QString( "CREATED" );
    if( inIpt == PT_DESCRIPTION )       return QString( "DESCRIPTION" );
    if( inIpt == PT_DTEND )             return QString( "DTEND" );
    if( inIpt == PT_DTSTAMP )           return QString( "DTSTAMP" );
    if( inIpt == PT_DTSTART )           return QString( "DTSTART" );
    if( inIpt == PT_DUE )               return QString( "DUE" );
    if( inIpt == PT_DURATION )          return QString( "DURATION" );
    if( inIpt == PT_EXDATE )            return QString( "EXDATE" );
    if( inIpt == PT_FREEBUSY )          return QString( "FREEBUSY" );
    if( inIpt == PT_GEO )               return QString( "GEO" );
    if( inIpt == PT_LAST_MODIFIED )     return QString( "LAST-MODIFIED" );
    if( inIpt == PT_LOCATION )          return QString( "LOCATION" );
    if( inIpt == PT_METHOD )            return QString( "METHOD" );
    if( inIpt == PT_ORGANIZER )         return QString( "ORGANIZER" );
    if( inIpt == PT_PERCENT_COMPLETE )  return QString( "PERCENT-COMPLETE" );
    if( inIpt == PT_PRIORITY )          return QString( "PRIORITY" );
    if( inIpt == PT_PRODID )            return QString( "PRODID" );
    if( inIpt == PT_RDATE )             return QString( "RDATE" );
    if( inIpt == PT_RECURRENCE_ID )     return QString( "RECURRENCE-ID" );
    if( inIpt == PT_RELATED_TO )        return QString( "RELATED-TO" );
    if( inIpt == PT_REPEAT )            return QString( "REPEAT" );
    if( inIpt == PT_REQUEST_STATUS )    return QString( "REQUEST-STATUS" );
    if( inIpt == PT_RESOURCES )         return QString( "RESOURCES" );
    if( inIpt == PT_RRULE )             return QString( "RRULE" );
    if( inIpt == PT_SEQUENCE )          return QString( "SEQUENCE" );
    if( inIpt == PT_STATUS )            return QString( "STATUS" );
    if( inIpt == PT_SUMMARY )           return QString( "SUMMARY" );
    if( inIpt == PT_TRANSP )            return QString( "TRANSP" );
    if( inIpt == PT_TRIGGER )           return QString( "TRIGGER" );
    if( inIpt == PT_TZID )              return QString( "TZID" );
    if( inIpt == PT_TZNAME )            return QString( "TZNAME" );
    if( inIpt == PT_TZOFFSETFROM )      return QString( "TZOFFSETFROM" );
    if( inIpt == PT_TZOFFSETTO )        return QString( "TZOFFSETTO" );
    if( inIpt == PT_TZURL )             return QString( "TZURL" );
    if( inIpt == PT_UID )               return QString( "UID" );
    if( inIpt == PT_URL )               return QString( "URL" );
    if( inIpt == PT_VERSION )           return QString( "VERSION" );
    // if( inIpt == PT_PROPERTY_UNKNOWN )
    return QString( "PROPERTY_UNKNOWN" );
}


Property::IcalPropertyType Property::propertyType( const QString inIptString ) const
{
    if( inIptString == "ACTION" )           return PT_ACTION;
    if( inIptString == "ATTACH" )           return PT_ATTACH;
    if( inIptString == "ATTENDEE" )         return PT_ATTENDEE;
    if( inIptString == "CALSCALE" )         return PT_CALSCALE;
    if( inIptString == "CATEGORIES" )       return PT_CATEGORIES;
    if( inIptString == "CLASS" )            return PT_CLASS;
    if( inIptString == "COMMENT" )          return PT_COMMENT;
    if( inIptString == "COMPLETED" )        return PT_COMPLETED;
    if( inIptString == "CONTACT" )          return PT_CONTACT;
    if( inIptString == "CREATED" )          return PT_CREATED;
    if( inIptString == "DESCRIPTION" )      return PT_DESCRIPTION;
    if( inIptString == "DTEND" )            return PT_DTEND;
    if( inIptString == "DTSTAMP" )          return PT_DTSTAMP;
    if( inIptString == "DTSTART" )          return PT_DTSTART;
    if( inIptString == "DUE" )              return PT_DUE;
    if( inIptString == "DURATION" )         return PT_DURATION;
    if( inIptString == "EXDATE" )           return PT_EXDATE;
    if( inIptString == "FREEBUSY" )         return PT_FREEBUSY;
    if( inIptString == "GEO" )              return PT_GEO;
    if( inIptString == "LAST-MODIFIED" )    return PT_LAST_MODIFIED;
    if( inIptString == "LOCATION" )         return PT_LOCATION;
    if( inIptString == "METHOD" )           return PT_METHOD;
    if( inIptString == "ORGANIZER" )        return PT_ORGANIZER;
    if( inIptString == "PERCENT-COMPLETE" ) return PT_PERCENT_COMPLETE;
    if( inIptString == "PRIORITY" )         return PT_PRIORITY;
    if( inIptString == "PRODID" )           return PT_PRODID;
    if( inIptString == "RDATE" )            return PT_RDATE;
    if( inIptString == "RECURRENCE-ID" )    return PT_RECURRENCE_ID;
    if( inIptString == "RELATED-TO" )       return PT_RELATED_TO;
    if( inIptString == "REPEAT" )           return PT_REPEAT;
    if( inIptString == "REQUEST-STATUS" )   return PT_REQUEST_STATUS;
    if( inIptString == "RESOURCES" )        return PT_RESOURCES;
    if( inIptString == "RRULE" )            return PT_RRULE;
    if( inIptString == "SEQUENCE" )         return PT_SEQUENCE;
    if( inIptString == "STATUS" )           return PT_STATUS;
    if( inIptString == "SUMMARY" )          return PT_SUMMARY;
    if( inIptString == "TRANSP" )           return PT_TRANSP;
    if( inIptString == "TRIGGER" )          return PT_TRIGGER;
    if( inIptString == "TZID" )             return PT_TZID;
    if( inIptString == "TZNAME" )           return PT_TZNAME;
    if( inIptString == "TZOFFSETFROM" )     return PT_TZOFFSETFROM;
    if( inIptString == "TZOFFSETTO" )       return PT_TZOFFSETTO;
    if( inIptString == "TZURL" )            return PT_TZURL;
    if( inIptString == "UID" )              return PT_UID;
    if( inIptString == "URL" )              return PT_URL;
    if( inIptString == "VERSION" )          return PT_VERSION;
    // if( inIptString == "PROPERTY_UNKNOWN" )
    return PT_PROPERTY_UNKNOWN;
}


void Property::splitParts( const QString inToSplit, QString &outPropName, QString &outArgument, QStringList &outParameters )
{
    QString input = inToSplit;
    QRegExp reg( "[;:]" );

    int index = input.indexOf( reg );

    if( index < 0 ) // no divider at all
    {
        outPropName = inToSplit;
        return;
    }
    outPropName = input.left( index ).toUpper();

    if( input[index] == ':' )   // the easy part foo:bar
    {
        // for RRULE:FREQ=DAILY;BYDAY=MO;COUNT=10, we would do something wrong here
        if( outPropName == "RRULE" )
        {
            QString arg = input.right( input.size() - index - 1);
            outParameters = arg.split( ';', QString::SkipEmptyParts );
            outArgument = "";
            return;
        }
        outArgument = input.right( input.size() - index - 1);
        return;
    }

    // properties available
    // foo;bar=xx;baz=123;hello="a;b;c":mailto:test@example.com

    // rest of string without prop name and divider
    QString rest = input.right( input.size() - index - 1);

    // [(bar=xx) , (baz=123), (hello="a), (b) (c":mailto:test@example.com)]
    QStringList paramListWithArg = rest.split( ';', QString::SkipEmptyParts );
    // [(bar=xx) , (baz=123), (hello="a;b;c":mailto:test@example.com)]
    QStringList resultList;  // correctly splitted list with arg inside
    while( not paramListWithArg.isEmpty() )
    {
        QString part = paramListWithArg.at( 0 );
        paramListWithArg.removeAt( 0 );
        // this part is for checking, if we split inside of strings
        while( ( not paramListWithArg.isEmpty() ) and ( part.count( '\"' ) % 2 != 0 ) )
        {
            QString x = paramListWithArg.at( 0 );
            paramListWithArg.removeAt( 0 );
            part += x;
        }
        resultList.append( part );
    }

    // last argument contains a param and the argument
    // lastList: [(hello="a;b;c") (mailto:test@example.com)]
    QStringList lastList = resultList.last().split( ':', QString::SkipEmptyParts );
    resultList.removeLast();

    // and again, check for strings
    QString aParam;
    while( not lastList.isEmpty() )
    {
        aParam += lastList.at( 0 );
        lastList.removeAt( 0 );
        if( aParam.count( '\"' ) % 2 == 0 )
            break;
    }
    // add param to the resultList
    resultList.append( aParam );

    // whatever comes now, must be an arg
    QString anArg = lastList.join( ':' );

    outArgument = anArg;            // can be empty
    outParameters = resultList;
}

