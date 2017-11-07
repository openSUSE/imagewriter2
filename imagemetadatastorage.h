#ifndef IMAGEMETADATASTORAGE_H
#define IMAGEMETADATASTORAGE_H

#include <cstdint>
#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QString>

class QXmlStreamReader;

/* This class contains information about the available images.
 * It is structured as a tree of decisions (which have options) and images as leaf nodes. */
class ImageMetadataStorage : public QAbstractItemModel
{
    Q_OBJECT

    /* Maximum depth of the decision tree. Is 0 on startup, changed to >= 1 after
     * an XML source was loaded. */
    Q_PROPERTY(unsigned int maxDepth READ getMaxDepth() NOTIFY maxDepthChanged())

public:
    /* Information about an Image. */
    struct Image {
        QString name;
        QString url;
        uint64_t size = 0;
        QString icon_url;
        QString icon_local_filename;
        QString sha256sum;
    };

    struct Decision;

    /* Information about an option. Contains either a decision or an image. */
    struct Option {
        QString name;
        QString icon_url;
        QString icon_local_filename;
        std::shared_ptr<Decision> decision;
        std::shared_ptr<Image> image;
    };

    /* Information about a decision. Contains name, the index of the preselected option
     * and all available options. */
    struct Decision {
        QString name;
        size_t preselected = 0;
        std::vector<Option> options;
    };

    /* Custom roles used in the Qt model */
    enum Roles {
        DecisionNameRole = Qt::UserRole,
        DecisionPreselectedOptionRole,
        OptionNameRole,
        OptionIconRole,
        ImageNameRole
    };

    Q_ENUM(Roles)

    ImageMetadataStorage();
    virtual ~ImageMetadataStorage();

    /* Call this once (and once only) to initialize the storage with the information
     * from the XML in xml_document. Returns false on failure. */
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

#endif // IMAGEMETADATASTORAGE_H
