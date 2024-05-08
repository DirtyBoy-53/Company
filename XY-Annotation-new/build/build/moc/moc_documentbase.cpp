/****************************************************************************
** Meta object code from reading C++ file 'documentbase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/canvas/documentbase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'documentbase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DocumentBase_t {
    QByteArrayData data[22];
    char stringdata0[223];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DocumentBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DocumentBase_t qt_meta_stringdata_DocumentBase = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DocumentBase"
QT_MOC_LITERAL(1, 13, 19), // "currentShapeChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 9), // "shapeName"
QT_MOC_LITERAL(4, 44, 11), // "sigAddShape"
QT_MOC_LITERAL(5, 56, 11), // "sigAddPoint"
QT_MOC_LITERAL(6, 68, 5), // "point"
QT_MOC_LITERAL(7, 74, 14), // "sigSetProperty"
QT_MOC_LITERAL(8, 89, 8), // "ShapePtr"
QT_MOC_LITERAL(9, 98, 5), // "shape"
QT_MOC_LITERAL(10, 104, 8), // "setScale"
QT_MOC_LITERAL(11, 113, 8), // "newScale"
QT_MOC_LITERAL(12, 122, 10), // "changeTask"
QT_MOC_LITERAL(13, 133, 11), // "task_mode_e"
QT_MOC_LITERAL(14, 145, 4), // "task"
QT_MOC_LITERAL(15, 150, 16), // "changeOperatMode"
QT_MOC_LITERAL(16, 167, 13), // "operat_mode_e"
QT_MOC_LITERAL(17, 181, 3), // "opt"
QT_MOC_LITERAL(18, 185, 14), // "changeDrawMode"
QT_MOC_LITERAL(19, 200, 11), // "draw_mode_e"
QT_MOC_LITERAL(20, 212, 4), // "draw"
QT_MOC_LITERAL(21, 217, 5) // "clean"

    },
    "DocumentBase\0currentShapeChanged\0\0"
    "shapeName\0sigAddShape\0sigAddPoint\0"
    "point\0sigSetProperty\0ShapePtr\0shape\0"
    "setScale\0newScale\0changeTask\0task_mode_e\0"
    "task\0changeOperatMode\0operat_mode_e\0"
    "opt\0changeDrawMode\0draw_mode_e\0draw\0"
    "clean"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DocumentBase[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    0,   62,    2, 0x06 /* Public */,
       5,    1,   63,    2, 0x06 /* Public */,
       7,    1,   66,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,   69,    2, 0x0a /* Public */,
      12,    1,   72,    2, 0x0a /* Public */,
      15,    1,   75,    2, 0x0a /* Public */,
      18,    1,   78,    2, 0x0a /* Public */,
      21,    0,   81,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPointF,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void,

       0        // eod
};

void DocumentBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DocumentBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentShapeChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->sigAddShape(); break;
        case 2: _t->sigAddPoint((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 3: _t->sigSetProperty((*reinterpret_cast< ShapePtr(*)>(_a[1]))); break;
        case 4: _t->setScale((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 5: _t->changeTask((*reinterpret_cast< task_mode_e(*)>(_a[1]))); break;
        case 6: _t->changeOperatMode((*reinterpret_cast< operat_mode_e(*)>(_a[1]))); break;
        case 7: _t->changeDrawMode((*reinterpret_cast< draw_mode_e(*)>(_a[1]))); break;
        case 8: _t->clean(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DocumentBase::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::currentShapeChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DocumentBase::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::sigAddShape)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DocumentBase::*)(const QPointF & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::sigAddPoint)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DocumentBase::*)(ShapePtr );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::sigSetProperty)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DocumentBase::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_DocumentBase.data,
    qt_meta_data_DocumentBase,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DocumentBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DocumentBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DocumentBase.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DocumentBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void DocumentBase::currentShapeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DocumentBase::sigAddShape()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DocumentBase::sigAddPoint(const QPointF & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DocumentBase::sigSetProperty(ShapePtr _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
