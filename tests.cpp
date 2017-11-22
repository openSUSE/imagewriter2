#include "imagemetadatastorage.h"

void testImageMetadataStorage()
{
    QString testXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
<decision name="Decision 0">
<option name="Option 0.0" preselected="true">
  <decision name="Decision 0.0">
      <option name="Option 0.0.0" preselected="true">
          <image url="url1" name="name1" size="42">
            <checksum type="sha256">195baca6c5f3b7f3ad4d7984a7f7bd5c4a37be2eb67e58b65d07ac3a2b599e83</checksum>
          </image>
      </option>
      <option name="Option 0.0.1">
          <image url="url2" name="Image 0.0.1" size="12">
            <checksum type="sha256">395baca6c5f3b7f3ad4d7984a7f7bd5c4a37be2eb67e58b65d07ac3a2b599e83</checksum>
          </image>
      </option>
    </decision>
</option>
</decision>)xml";

    ImageMetadataStorage ims;
    Q_ASSERT(ims.readFromXML(testXML));
    Q_ASSERT(ims.getMaxDepth() == 2);
    auto decision0 = QModelIndex{};
    Q_ASSERT(ims.data(decision0, ImageMetadataStorage::DecisionNameRole) == QStringLiteral("Decision 0"));
    Q_ASSERT(ims.data(decision0, ImageMetadataStorage::DecisionPreselectedOptionRole) == 0);
    auto option00 = ims.index(0, 0, decision0);
    Q_ASSERT(option00.isValid());
    Q_ASSERT(option00.data(ImageMetadataStorage::OptionNameRole) == QStringLiteral("Option 0.0"));
    Q_ASSERT(option00.data(ImageMetadataStorage::DecisionNameRole) == QStringLiteral("Decision 0.0"));
    auto option001 = ims.index(1, 0, option00);
    Q_ASSERT(option001.data(ImageMetadataStorage::ImageNameRole) == QStringLiteral("Image 0.0.1"));
    Q_ASSERT(option001.data(ImageMetadataStorage::ImageDataRole).value<ImageMetadataStorage::Image>().sha256sum
             == QStringLiteral("395baca6c5f3b7f3ad4d7984a7f7bd5c4a37be2eb67e58b65d07ac3a2b599e83"));
}

void runAllTests()
{
    testImageMetadataStorage();
}
