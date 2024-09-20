/****************************************************************************
** Meta object code from reading C++ file 'MulWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Src/Window/MulWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MulWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MulWindow_t {
    QByteArrayData data[20];
    char stringdata0[158];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MulWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MulWindow_t qt_meta_stringdata_MulWindow = {
    {
QT_MOC_LITERAL(0, 0, 9), // "MulWindow"
QT_MOC_LITERAL(1, 10, 9), // "setLayout"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 3), // "row"
QT_MOC_LITERAL(4, 25, 3), // "col"
QT_MOC_LITERAL(5, 29, 10), // "saveLayout"
QT_MOC_LITERAL(6, 40, 13), // "restoreLayout"
QT_MOC_LITERAL(7, 54, 10), // "mergeCells"
QT_MOC_LITERAL(8, 65, 2), // "lt"
QT_MOC_LITERAL(9, 68, 2), // "rb"
QT_MOC_LITERAL(10, 71, 13), // "exchangeCells"
QT_MOC_LITERAL(11, 85, 12), // "VideoWidget*"
QT_MOC_LITERAL(12, 98, 7), // "player1"
QT_MOC_LITERAL(13, 106, 7), // "player2"
QT_MOC_LITERAL(14, 114, 7), // "stretch"
QT_MOC_LITERAL(15, 122, 8), // "QWidget*"
QT_MOC_LITERAL(16, 131, 3), // "wdg"
QT_MOC_LITERAL(17, 135, 9), // "onSendMsg"
QT_MOC_LITERAL(18, 145, 8), // "QString&"
QT_MOC_LITERAL(19, 154, 3) // "msg"

    },
    "MulWindow\0setLayout\0\0row\0col\0saveLayout\0"
    "restoreLayout\0mergeCells\0lt\0rb\0"
    "exchangeCells\0VideoWidget*\0player1\0"
    "player2\0stretch\0QWidget*\0wdg\0onSendMsg\0"
    "QString&\0msg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MulWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   49,    2, 0x0a /* Public */,
       5,    0,   54,    2, 0x0a /* Public */,
       6,    0,   55,    2, 0x0a /* Public */,
       7,    2,   56,    2, 0x0a /* Public */,
      10,    2,   61,    2, 0x0a /* Public */,
      14,    1,   66,    2, 0x0a /* Public */,
      17,    1,   69,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    8,    9,
    QMetaType::Void, 0x80000000 | 11, 0x80000000 | 11,   12,   13,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18,   19,

       0        // eod
};

void MulWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MulWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setLayout((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->saveLayout(); break;
        case 2: _t->restoreLayout(); break;
        case 3: _t->mergeCells((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: _t->exchangeCells((*reinterpret_cast< VideoWidget*(*)>(_a[1])),(*reinterpret_cast< VideoWidget*(*)>(_a[2]))); break;
        case 5: _t->stretch((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 6: _t->onSendMsg((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MulWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MulWindow.data,
    qt_meta_data_MulWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MulWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MulWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MulWindow.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MulWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
