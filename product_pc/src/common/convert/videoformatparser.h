#ifndef VIDEOFORMATPARSER_H
#define VIDEOFORMATPARSER_H
#include <QByteArray>


class VideoFormatParser
{
public:
    VideoFormatParser();

    static void rawToRgb(QByteArray &rawData, QByteArray &rgbData);
};

#endif // VIDEOFORMATPARSER_H
