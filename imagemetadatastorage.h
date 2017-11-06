#ifndef IMAGEMETADATASTORAGE_H
#define IMAGEMETADATASTORAGE_H

#include <cstdint>
#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QString>
#include <QUrl>

class QXmlStreamReader;

class ImageMetadataStorage : public QAbstractItemModel
{
    Q_OBJECT

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

    enum Roles {
        DecisionNameRole = Qt::UserRole,
        ChoiceNameRole,
        ChoiceIconRole,
        ImageNameRole
    };

    Q_ENUM(Roles)

    ImageMetadataStorage(QUrl rootUrl);
    virtual ~ImageMetadataStorage();

    bool readFromXML(QString xml_document);

    Decision const * getRoot();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    bool parseDecision(Decision &decision, QXmlStreamReader &reader);
    bool parseChoice(Choice &choice, QXmlStreamReader &reader);
    bool parseImage(Image &image, QXmlStreamReader &reader);

    QUrl rootUrl;
    Choice root;
};

Q_DECLARE_METATYPE(ImageMetadataStorage::Choice)

#endif // IMAGEMETADATASTORAGE_H
