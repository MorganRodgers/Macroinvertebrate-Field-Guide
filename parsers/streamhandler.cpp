#include "streamhandler.h"

StreamHandler::StreamHandler()
{
    curlyBraceElement.setPattern("{{(.+?)}}");
    curlyBraceElement.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
}

Stream StreamHandler::parse(const QString &text)
{
    buffer.clear();
    buffer.append(text);
    Stream stream;
    bool seenInfoBox = false;

    QRegularExpressionMatchIterator iter = curlyBraceElement.globalMatch(buffer);
    while(iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString matchingString = match.captured(1);
        if(!seenInfoBox && matchingString.contains("Infobox stream")) {
            // Handle Infobox stream
            parseInfoboxToStream(matchingString, stream);
            seenInfoBox = true;
        } else if(seenInfoBox) {
            // Bug List?
            stream.invertebrateList.append(matchingString);
        } else {
            // Should be nothing to do
//            qDebug() << matchingString;
        }
    }

    return stream;
}

void StreamHandler::parseInfoboxToStream(const QString &infoBox, Stream &stream)
{
    QString buffer(infoBox);
    buffer = buffer.replace("Infobox stream ", "");
    for(QString line: buffer.split("\n", QString::SkipEmptyParts)) {
        QStringList pair = line.split(QRegularExpression("\\s*=\\s*"));
        if(pair.length() != 2) {
//            qDebug() << "Pair does not contain 2 elements" << pair;
        } else {
            QString key = pair.at(0).trimmed();
            QString value = pair.at(1).trimmed();

            if(key == "|Country") {
                stream.country = value;
            } else if(key == "|Stream") {
                stream.title = value;
            } else if(key == "|Latitude") {
                bool convertedOk = true;
                stream.latitude = value.toDouble(&convertedOk);

                if(!convertedOk) {
//                    qDebug() << "String to double conversion error for: " << value;
                    stream.latitude = 0.0;
                }
            } else if(key == "|Longitude") {
                bool convertedOk = true;
                stream.longitude = value.toDouble(&convertedOk);

                if(!convertedOk) {
//                    qDebug() << "String to double conversion error for: " << value;
                    stream.longitude = 0.0;
                }
            } else if(key == "|State or Province") {
                stream.state_or_province = value;
            } else {
//                qDebug() << "Key: " << key.trimmed() << " Value: " << value;
            }
        }
    }
}

StreamHandler::~StreamHandler()
{}
