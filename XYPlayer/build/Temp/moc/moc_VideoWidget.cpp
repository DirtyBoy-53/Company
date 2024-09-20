/****************************************************************************
** Meta object code from reading C++ file 'VideoWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Src/Window/VideoWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VideoWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VideoWidget_t {
    QByteArrayData data[27];
    char stringdata0[255];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoWidget_t qt_meta_stringdata_VideoWidget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "VideoWidget"
QT_MOC_LITERAL(1, 12, 10), // "sigSendMsg"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 8), // "QString&"
QT_MOC_LITERAL(4, 33, 3), // "msg"
QT_MOC_LITERAL(5, 37, 4), // "open"
QT_MOC_LITERAL(6, 42, 6), // "Media&"
QT_MOC_LITERAL(7, 49, 5), // "media"
QT_MOC_LITERAL(8, 55, 5), // "close"
QT_MOC_LITERAL(9, 61, 5), // "start"
QT_MOC_LITERAL(10, 67, 4), // "stop"
QT_MOC_LITERAL(11, 72, 5), // "pause"
QT_MOC_LITERAL(12, 78, 6), // "resume"
QT_MOC_LITERAL(13, 85, 7), // "restart"
QT_MOC_LITERAL(14, 93, 5), // "retry"
QT_MOC_LITERAL(15, 99, 13), // "onTimerUpdate"
QT_MOC_LITERAL(16, 113, 13), // "onOpenSucceed"
QT_MOC_LITERAL(17, 127, 12), // "onOpenFailed"
QT_MOC_LITERAL(18, 140, 13), // "onPlayerError"
QT_MOC_LITERAL(19, 154, 14), // "setAspectRatio"
QT_MOC_LITERAL(20, 169, 14), // "aspect_ratio_t"
QT_MOC_LITERAL(21, 184, 2), // "ar"
QT_MOC_LITERAL(22, 187, 7), // "I2CTest"
QT_MOC_LITERAL(23, 195, 8), // "onUpdate"
QT_MOC_LITERAL(24, 204, 17), // "onUpdateWatermark"
QT_MOC_LITERAL(25, 222, 27), // "QList<CustomWatermarkInfo>&"
QT_MOC_LITERAL(26, 250, 4) // "list"

    },
    "VideoWidget\0sigSendMsg\0\0QString&\0msg\0"
    "open\0Media&\0media\0close\0start\0stop\0"
    "pause\0resume\0restart\0retry\0onTimerUpdate\0"
    "onOpenSucceed\0onOpenFailed\0onPlayerError\0"
    "setAspectRatio\0aspect_ratio_t\0ar\0"
    "I2CTest\0onUpdate\0onUpdateWatermark\0"
    "QList<CustomWatermarkInfo>&\0list"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   99,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,  102,    2, 0x0a /* Public */,
       8,    0,  105,    2, 0x0a /* Public */,
       9,    0,  106,    2, 0x0a /* Public */,
      10,    0,  107,    2, 0x0a /* Public */,
      11,    0,  108,    2, 0x0a /* Public */,
      12,    0,  109,    2, 0x0a /* Public */,
      13,    0,  110,    2, 0x0a /* Public */,
      14,    0,  111,    2, 0x0a /* Public */,
      15,    0,  112,    2, 0x0a /* Public */,
      16,    0,  113,    2, 0x0a /* Public */,
      17,    0,  114,    2, 0x0a /* Public */,
      18,    0,  115,    2, 0x0a /* Public */,
      19,    1,  116,    2, 0x0a /* Public */,
      22,    0,  119,    2, 0x0a /* Public */,
      23,    0,  120,    2, 0x0a /* Public */,
      24,    1,  121,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25,   26,

       0        // eod
};

void VideoWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigSendMsg((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->open((*reinterpret_cast< Media(*)>(_a[1]))); break;
        case 2: _t->close(); break;
        case 3: _t->start(); break;
        case 4: _t->stop(); break;
        case 5: _t->pause(); break;
        case 6: _t->resume(); break;
        case 7: _t->restart(); break;
        case 8: _t->retry(); break;
        case 9: _t->onTimerUpdate(); break;
        case 10: _t->onOpenSucceed(); break;
        case 11: _t->onOpenFailed(); break;
        case 12: _t->onPlayerError(); break;
        case 13: _t->setAspectRatio((*reinterpret_cast< aspect_ratio_t(*)>(_a[1]))); break;
        case 14: { bool _r = _t->I2CTest();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->onUpdate(); break;
        case 16: _t->onUpdateWatermark((*reinterpret_cast< QList<CustomWatermarkInfo>(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoWidget::*)(QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoWidget::sigSendMsg)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VideoWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_VideoWidget.data,
    qt_meta_data_VideoWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoWidget.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int VideoWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void VideoWidget::sigSendMsg(QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_VideoResMonitor_t {
    QByteArrayData data[5];
    char stringdata0[55];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoResMonitor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoResMonitor_t qt_meta_stringdata_VideoResMonitor = {
    {
QT_MOC_LITERAL(0, 0, 15), // "VideoResMonitor"
QT_MOC_LITERAL(1, 16, 18), // "sigVideoResChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 12), // "sigParamLine"
QT_MOC_LITERAL(4, 49, 5) // "param"

    },
    "VideoResMonitor\0sigVideoResChanged\0\0"
    "sigParamLine\0param"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoResMonitor[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,
       3,    1,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    4,

       0        // eod
};

void VideoResMonitor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoResMonitor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigVideoResChanged(); break;
        case 1: _t->sigParamLine((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoResMonitor::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoResMonitor::sigVideoResChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoResMonitor::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoResMonitor::sigParamLine)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VideoResMonitor::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_VideoResMonitor.data,
    qt_meta_data_VideoResMonitor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoResMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoResMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoResMonitor.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int VideoResMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void VideoResMonitor::sigVideoResChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void VideoResMonitor::sigParamLine(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
