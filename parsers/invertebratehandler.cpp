#include "invertebratehandler.h"

InvertebrateHandler::InvertebrateHandler()
{
    curlyBraceElement.setPattern("{{InsectSection(.+?)}}");
    curlyBraceElement.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    curlyBraceElement.optimize();

    textBlock.setPattern("\\|text\\s*=\\s*(.+?)<!--Stop-->");
    textBlock.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    textBlock.optimize();

    textBlockWithoutStop.setPattern("\\|text\\s*=\\s*(.+?)$");
    textBlockWithoutStop.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    textBlockWithoutStop.optimize();

    wikiStyleLink.setPattern("\\[http[^ ]+ ([^]]+)\\]");
    wikiStyleLink.optimize();

    tooMuchWhiteSpace.setPattern("[\t\f ]{2,}");
    tooMuchWhiteSpace.optimize();
}

void InvertebrateHandler::parseInfoboxToInvertebrate(const QString &infoBox, Invertebrate &invertebrate)
{
    QRegularExpressionMatch match = textBlock.match(infoBox);
    if(match.hasMatch()) {
        invertebrate.description = match.captured(1).trimmed();
    } else {
        match = textBlockWithoutStop.match(infoBox);
        if(match.hasMatch()) {
            invertebrate.description = match.captured(1).trimmed().replace(tooMuchWhiteSpace, " ");
        } else {
//            qDebug() << "Something went wrong with the match: " << infoBox;
        }
    }

    fixWikiLinks(invertebrate);

    for(QString line: infoBox.split("\n", QString::SkipEmptyParts)) {
        QStringList pair = line.split(QRegularExpression(" ?= ?"));
        if(pair.length() != 2) {
//            qDebug() << "Pair does not contain 2 elements" << pair;
        } else {
            QString key = pair.at(0).trimmed();
            QString value = pair.at(1).trimmed();

            if(key == "|name") {
                invertebrate.name = naiveStringToTitleCase(value);
            } else if(key == "|common name") {
                invertebrate.commonName = naiveStringToTitleCase(value);
            } else if(key == "|family") {
                invertebrate.family = naiveStringToTitleCase(value);
            } else if(key == "|tied fly name") {
                invertebrate.flyName = naiveStringToTitleCase(value);
            } else if(key == "|genus") {
                invertebrate.genus = naiveStringToTitleCase(value);
            } else if(key == "|image") {
                invertebrate.imageFileRemote = value;
            } else if(key == "|order") {
                invertebrate.order = naiveStringToTitleCase(value);
            }
        }
    }

#ifndef QT_NO_DEBUG_OUTPUT
//    if(!validate(invertebrate)) {
//        qDebug() << "INFOBOX FOLLOWS" << "\n" << infoBox;
//    }
#endif
}

bool InvertebrateHandler::validate(const Invertebrate &invertebrate)
{
    QStringList nullAttributes;

//    commonName is commonly missing and may not be an error?
    if(invertebrate.commonName.isEmpty()) {
        nullAttributes << "commonName";
    }

    if(invertebrate.description.isEmpty()) {
        nullAttributes << "description/title";
    }

    if(invertebrate.imageFileRemote.isEmpty()) {
        nullAttributes << "image";
    }

    if(invertebrate.family.isEmpty()) {
        nullAttributes << "family";
    }

    if(invertebrate.genus.isEmpty()) {
        nullAttributes << "genus";
    }

    if(invertebrate.name.isEmpty()) {
        nullAttributes << "name";
    }

    if(invertebrate.order.isEmpty()) {
        nullAttributes << "family";
    }

    if(nullAttributes.count() > 0) {
//        qDebug() << "INVALID INVERTEBRATE: " << invertebrate.name << " IS MISSING: " << nullAttributes;
        return false;
    }

    return true;
}

Invertebrate InvertebrateHandler::parse(const QString &text)
{
    Invertebrate invertebrate;

    QRegularExpressionMatchIterator iter = curlyBraceElement.globalMatch(text);
    while(iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString matchingString = match.captured(1).replace("''", "\"");

        parseInfoboxToInvertebrate(matchingString, invertebrate);
    }

    return invertebrate;
}

InvertebrateHandler::~InvertebrateHandler()
{}

void InvertebrateHandler::fixWikiLinks(Invertebrate &invertebrate)
{
    QRegularExpressionMatchIterator iter = wikiStyleLink.globalMatch(invertebrate.description);
    while(iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        invertebrate.description = invertebrate.description.replace(match.captured(0), match.captured(1));
    }
}

QString InvertebrateHandler::naiveStringToTitleCase(const QString &original)
{
    QString string = original.toLower();
    bool lastCharWasSpace = true;
    for(int i = 0; i < string.length(); i++) {
        QCharRef c = string[i];
        if(c.isSpace()) {
            lastCharWasSpace = true;
        } else {
            if(lastCharWasSpace) {
                string[i] = c.toUpper();
            }
            lastCharWasSpace = false;
        }
    }

    return string;
}
