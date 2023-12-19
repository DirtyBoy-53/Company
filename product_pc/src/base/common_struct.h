#ifndef COMMON_STRUCT_H
#define COMMON_STRUCT_H

namespace myCommon{
enum DLParity{
	None = 0,
	Even,
	Odd
};
}

enum DataStreamFormat{
    FormatHex = 0,
    FormatString
};

typedef struct _tagDef {
    int x;
    int y;
}POINTX;
#endif // COMMON_STRUCT_H
