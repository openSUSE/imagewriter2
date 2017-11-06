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
            {
                root.decision = std::make_shared<Decision>();
                failed = !parseDecision(*root.decision, reader);
            }
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
    return root.decision.get();
}

QModelIndex ImageMetadataStorage::index(int row, int column, const QModelIndex &parent) const
{
    Choice *choice = parent.isValid() ? reinterpret_cast<Choice*>(parent.internalPointer()) : const_cast<Choice*>(&root);

    if(!choice->decision || static_cast<size_t>(row) >= choice->decision->choices.size())
        return QModelIndex();

    return createIndex(row, column, &choice->decision->choices[static_cast<size_t>(row)]);
}

QVariant ImageMetadataStorage::data(const QModelIndex &index, int role) const
{
    Choice *choice = index.isValid() ? reinterpret_cast<Choice*>(index.internalPointer()) : const_cast<Choice*>(&root);

    switch(role)
    {
    case DecisionNameRole:
        return choice->decision ? choice->decision->name : QVariant{};
    case ChoiceNameRole:
        return choice->name;
    case ChoiceIconRole:
        return choice->icon_local_filename;
    case ImageNameRole:
        return choice->image ? choice->image->name : QVariant{};
    default:
        return QVariant{};
    }
}

QModelIndex ImageMetadataStorage::parent(const QModelIndex &index) const
{
    qDebug() << index;
    return QModelIndex{};
}

int ImageMetadataStorage::rowCount(const QModelIndex &parent) const
{
    Choice *choice = parent.isValid() ? reinterpret_cast<Choice*>(parent.internalPointer()) : const_cast<Choice*>(&root);

    return choice->decision ? static_cast<int>(choice->decision->choices.size()) : 0;
}

int ImageMetadataStorage::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 1;
}

QHash<int, QByteArray> ImageMetadataStorage::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DecisionNameRole] = "DecisionName";
    roles[ChoiceNameRole] = "ChoiceName";
    roles[ChoiceIconRole] = "ChoiceIcon";
    roles[ImageNameRole] = "ImageName";
    return roles;
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
