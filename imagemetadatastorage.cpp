#include <QDebug>
#include <QXmlStreamReader>

#include "imagemetadatastorage.h"

ImageMetadataStorage::ImageMetadataStorage(QUrl rootUrl)
    : rootUrl(rootUrl)
{}

ImageMetadataStorage::~ImageMetadataStorage()
{

}

bool ImageMetadataStorage::readFromXML(QString xml_document)
{
    QXmlStreamReader reader;
    reader.addData(xml_document);

    bool failed = false;

    while(!failed && !reader.atEnd())
    {
        if(reader.hasError())
        {
            failed = true;
            break;
        }

        switch(reader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if(reader.name() == QStringLiteral("decision"))
                failed = !parseDecision(root, reader);
            else
                failed = true;

            break;
        default:
            break;
        }
    }

    return !failed;
}

const ImageMetadataStorage::Decision *ImageMetadataStorage::getRoot()
{
    return &root;
}

bool ImageMetadataStorage::parseDecision(ImageMetadataStorage::Decision &decision, QXmlStreamReader &reader)
{
    auto attrs = reader.attributes();

    if(!attrs.hasAttribute(QStringLiteral("name")))
        return false;

    decision.name = attrs.value(QStringLiteral("name")).toString();

    bool failed = false;
    while(!failed && !reader.atEnd())
    {
        switch(reader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if(reader.name() == QStringLiteral("choice"))
            {
                if(reader.attributes().value(QStringLiteral("preselected")) == QStringLiteral("true"))
                    decision.preselected = decision.choices.size();

                decision.choices.resize(decision.choices.size() + 1);
                failed = !parseChoice(*decision.choices.rbegin(), reader);
            }
            else
                failed = true;

            break;
        case QXmlStreamReader::EndElement:
            return true;
        default:
            break;
        }
    }

    return !failed;
}

bool ImageMetadataStorage::parseChoice(ImageMetadataStorage::Choice &choice, QXmlStreamReader &reader)
{
    auto attrs = reader.attributes();

    if(!attrs.hasAttribute(QStringLiteral("name")))
        return false;

    choice.name = attrs.value(QStringLiteral("name")).toString();

    if(attrs.hasAttribute(QStringLiteral("icon")))
        choice.icon_url = attrs.value(QStringLiteral("icon")).toString();

    bool failed = false;
    while(!failed && !reader.atEnd())
    {
        switch(reader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if(reader.name() == QStringLiteral("image"))
            {
                if(choice.decision)
                    failed = true;
                else
                {
                    choice.image = std::make_shared<ImageMetadataStorage::Image>();
                    failed = !parseImage(*choice.image, reader);
                }
            }
            else if(reader.name() == QStringLiteral("decision"))
            {
                if(choice.image)
                    failed = true;
                else
                {
                    choice.decision = std::make_shared<ImageMetadataStorage::Decision>();
                    failed = !parseDecision(*choice.decision, reader);
                }
            }
            else
                failed = true;

            break;
        case QXmlStreamReader::EndElement:
            return true;
        default:
            break;
        }
    }

    return !failed;
}

bool ImageMetadataStorage::parseImage(ImageMetadataStorage::Image &image, QXmlStreamReader &reader)
{
    auto attrs = reader.attributes();

    if(!attrs.hasAttribute(QStringLiteral("name"))
       || !attrs.hasAttribute(QStringLiteral("url"))
       || !attrs.hasAttribute(QStringLiteral("size")))
        return false;

    image.name = attrs.value(QStringLiteral("name")).toString();
    image.url = attrs.value(QStringLiteral("url")).toString();

    bool ok;
    auto size = attrs.value(QStringLiteral("size")).toLongLong(&ok);

    if(!ok || size < 0)
        return false;

    image.size = static_cast<decltype(image.size)>(size);

    if(attrs.hasAttribute(QStringLiteral("icon")))
        image.icon_url = attrs.value(QStringLiteral("icon")).toString();

    bool failed = false;
    while(!failed && !reader.atEnd())
    {
        switch(reader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if(reader.name() == QStringLiteral("checksum"))
            {
                if(reader.attributes().value(QStringLiteral("type")) == QStringLiteral("sha256"))
                    image.sha256sum = reader.readElementText(QXmlStreamReader::IncludeChildElements);
            }
            else
                failed = true;

            break;
        case QXmlStreamReader::EndElement:
            return true;
        default:
            break;
        }
    }

    return !failed;
}
