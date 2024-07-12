#ifndef COMMON_H
#define COMMON_H

//#include "cubeannotationitem.h"

#include <QList>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QString>

#include <cmath>
#include <map>

namespace ColorUtils{
extern QList<QColor> randomColors(int count);
extern QColor randomColor();

extern QIcon iconFromColor(QColor color, QSize size = QSize(16,16));
}

namespace StringConstants{

    // for single image
    const QString SUFFIX_DET_LABEL_ANNO("_detect_labels_annotations.json");
    const QString SUFFIX_SEG_LABEL_ANNO("_segment_labels_annotations.json");

    // for multiple image
    const QString FILENAME_DIR_LABEL("_labels.json");
    const QString SUFFIX_DET_ANNO("_detect_annotations.json");
    const QString SUFFIX_SEG_ANNO("_segment_annotations.json");

    // for 3d image
    const QString FILENAME_DET3D_LABEL_ANNO("detect3d_labels_annotations.json");
    const QString FILENAME_SEG3D_LABEL_ANNO("segment3d_labels_annotations.json");

    // for image result
    const QString SUFFIX_SEG_COLOR("_segment_color.png");
    const QString SUFFIX_SEG_LABELID("_segment_labelId.png");
    const QString SUFFIX_SEG3D_COLOR("_segment3d_color.png");
    const QString SUFFIX_SEG3D_LABELID("_segment3d_labelId.png");
}
#endif // COMMON_H
