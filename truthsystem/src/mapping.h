#ifndef MAPPING_H
#define MAPPING_H


class Mapping
{
public:
    Mapping();

    bool Data16ToData8(short *srcData, unsigned char *destData, int size);

    bool Data16ToRGB24(short *srcData, unsigned char *destData, int size, int palette);

    bool YUVToRGB24(short *srcData, unsigned char *destData, int width, int height);
};

#endif // MAPPING_H
