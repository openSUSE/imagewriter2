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

    Q_PROPERTY(unsigned int maxDepth READ getMaxDepth() NOTIFY maxDepthChanged())

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

    struct Option {
        QString name;
        QString icon_url;
        QString icon_local_filename;
        std::shared_ptr<Decision> decision;
        std::shared_ptr<Image> image;
    };

    struct Decision {
        QString name;
        size_t preselected = 0;
        std::vector<Option> options;
    };

    enum Roles {
        DecisionNameRole = Qt::UserRole,
        OptionNameRole,
        OptionIconRole,
        ImageNameRole
    };

    Q_ENUM(Roles)

    ImageMetadataStorage();
    virtual ~ImageMetadataStorage();

    Q_INVOKABLE bool readFromXML(QString xml_document);

    Decision const * getRoot();
    unsigned int getMaxDepth();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void maxDepthChanged();

private:
    bool parseDecision(Decision &decision, QXmlStreamReader &reader);
    bool parseOption(Option &option, QXmlStreamReader &reader);
    bool parseImage(Image &image, QXmlStreamReader &reader);

    int maxDepth = 0;
    int currentDepth = 0;
    Option root;
};

Q_DECLARE_METATYPE(ImageMetadataStorage::Option)

#endif // IMAGEMETADATASTORAGE_H
