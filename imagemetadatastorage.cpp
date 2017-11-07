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
    Option *option = parent.isValid() ? reinterpret_cast<Option*>(parent.internalPointer()) : const_cast<Option*>(&root);

    if(!option->decision || static_cast<size_t>(row) >= option->decision->options.size())
        return QModelIndex();

    return createIndex(row, column, &option->decision->options[static_cast<size_t>(row)]);
}

QVariant ImageMetadataStorage::data(const QModelIndex &index, int role) const
{
    Option *option = index.isValid() ? reinterpret_cast<Option*>(index.internalPointer()) : const_cast<Option*>(&root);

    switch(role)
    {
    case DecisionNameRole:
        return option->decision ? option->decision->name : QVariant{};
    case OptionNameRole:
        return option->name;
    case OptionIconRole:
        return option->icon_local_filename;
    case ImageNameRole:
        return option->image ? option->image->name : QVariant{};
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
    Option *option = parent.isValid() ? reinterpret_cast<Option*>(parent.internalPointer()) : const_cast<Option*>(&root);

    return option->decision ? static_cast<int>(option->decision->options.size()) : 0;
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
    roles[OptionNameRole] = "OptionName";
    roles[OptionIconRole] = "OptionIcon";
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
            if(reader.name() == QStringLiteral("option"))
            {
                if(reader.attributes().value(QStringLiteral("preselected")) == QStringLiteral("true"))
                    decision.preselected = decision.options.size();

                decision.options.resize(decision.options.size() + 1);
                failed = !parseOption(*decision.options.rbegin(), reader);
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

bool ImageMetadataStorage::parseOption(ImageMetadataStorage::Option &option, QXmlStreamReader &reader)
{
    auto attrs = reader.attributes();

    if(!attrs.hasAttribute(QStringLiteral("name")))
        return false;

    option.name = attrs.value(QStringLiteral("name")).toString();

    if(attrs.hasAttribute(QStringLiteral("icon")))
        option.icon_url = attrs.value(QStringLiteral("icon")).toString();

    bool failed = false;
    while(!failed && !reader.atEnd())
    {
        switch(reader.readNext())
        {
        case QXmlStreamReader::StartElement:
            if(reader.name() == QStringLiteral("image"))
            {
                if(option.decision)
                    failed = true;
                else
                {
                    option.image = std::make_shared<ImageMetadataStorage::Image>();
                    failed = !parseImage(*option.image, reader);
                }
            }
            else if(reader.name() == QStringLiteral("decision"))
            {
                if(option.image)
                    failed = true;
                else
                {
                    option.decision = std::make_shared<ImageMetadataStorage::Decision>();
                    failed = !parseDecision(*option.decision, reader);
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
