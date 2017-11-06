#ifndef IMAGEMETADATASTORAGE_H
#define IMAGEMETADATASTORAGE_H

#include <cstdint>
#include <memory>
#include <vector>

#include <QObject>
#include <QString>
#include <QUrl>

class QXmlStreamReader;

class ImageMetadataStorage : public QObject
{
public:
    struct Image {
        QString name;
        QString url;
        uint64_t size = 0;
        QString icon_url;
        QString icon_local_filename;
        QString sha256sum;
    };

    struct Decision;

    struct Choice {
        QString name;
        QString icon_url;
        QString icon_local_filename;
        std::shared_ptr<Decision> decision;
        std::shared_ptr<Image> image;
    };

    struct Decision {
        QString name;
        size_t preselected = 0;
        std::vector<Choice> choices;
    };

    ImageMetadataStorage(QUrl rootUrl);
    virtual ~ImageMetadataStorage();

    bool readFromXML(QString xml_document);

    Decision const * getRoot();

private:
    bool parseDecision(Decision &decision, QXmlStreamReader &reader);
    bool parseChoice(Choice &choice, QXmlStreamReader &reader);
    bool parseImage(Image &image, QXmlStreamReader &reader);

    QUrl rootUrl;
    Decision root;
};

Q_DECLARE_METATYPE(ImageMetadataStorage::Choice)

#endif // IMAGEMETADATASTORAGE_H
