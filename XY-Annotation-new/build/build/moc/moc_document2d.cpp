/****************************************************************************
** Meta object code from reading C++ file 'document2d.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/canvas/document2d.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'document2d.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Document2D_t {
    QByteArrayData data[14];
    char stringdata0[131];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Document2D_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Document2D_t qt_meta_stringdata_Document2D = {
    {
QT_MOC_LITERAL(0, 0, 10), // "Document2D"
QT_MOC_LITERAL(1, 11, 8), // "setScale"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 8), // "newScale"
QT_MOC_LITERAL(4, 30, 10), // "changeTask"
QT_MOC_LITERAL(5, 41, 11), // "task_mode_e"
QT_MOC_LITERAL(6, 53, 4), // "task"
QT_MOC_LITERAL(7, 58, 16), // "changeOperatMode"
QT_MOC_LITERAL(8, 75, 13), // "operat_mode_e"
QT_MOC_LITERAL(9, 89, 3), // "opt"
QT_MOC_LITERAL(10, 93, 14), // "changeDrawMode"
QT_MOC_LITERAL(11, 108, 11), // "draw_mode_e"
QT_MOC_LITERAL(12, 120, 4), // "draw"
QT_MOC_LITERAL(13, 125, 5) // "clean"

    },
    "Document2D\0setScale\0\0newScale\0changeTask\0"
    "task_mode_e\0task\0changeOperatMode\0"
    "operat_mode_e\0opt\0changeDrawMode\0"
    "draw_mode_e\0draw\0clean"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Document2D[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x0a /* Public */,
       4,    1,   42,    2, 0x0a /* Public */,
       7,    1,   45,    2, 0x0a /* Public */,
      10,    1,   48,    2, 0x0a /* Public */,
      13,    0,   51,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,

       0        // eod
};

void Document2D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Document2D *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setScale((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 1: _t->changeTask((*reinterpret_cast< task_mode_e(*)>(_a[1]))); break;
        case 2: _t->changeOperatMode((*reinterpret_cast< operat_mode_e(*)>(_a[1]))); break;
        case 3: _t->changeDrawMode((*reinterpret_cast< draw_mode_e(*)>(_a[1]))); break;
        case 4: _t->clean(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Document2D::staticMetaObject = { {
    QMetaObject::SuperData::link<DocumentBase::staticMetaObject>(),
    qt_meta_stringdata_Document2D.data,
    qt_meta_data_Document2D,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Document2D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Document2D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Document2D.stringdata0))
        return static_cast<void*>(this);
    return DocumentBase::qt_metacast(_clname);
}

int Document2D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DocumentBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
