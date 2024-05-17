/****************************************************************************
** Meta object code from reading C++ file 'CanvasBase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/Canvas/CanvasBase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CanvasBase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CanvasBase_t {
    QByteArrayData data[13];
    char stringdata0[122];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CanvasBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CanvasBase_t qt_meta_stringdata_CanvasBase = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CanvasBase"
QT_MOC_LITERAL(1, 11, 18), // "sigCurShapeChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 9), // "shapeName"
QT_MOC_LITERAL(4, 41, 11), // "sigAddShape"
QT_MOC_LITERAL(5, 53, 11), // "sigAddPoint"
QT_MOC_LITERAL(6, 65, 5), // "point"
QT_MOC_LITERAL(7, 71, 14), // "sigSetProperty"
QT_MOC_LITERAL(8, 86, 8), // "ShapePtr"
QT_MOC_LITERAL(9, 95, 5), // "shape"
QT_MOC_LITERAL(10, 101, 5), // "clean"
QT_MOC_LITERAL(11, 107, 6), // "zoomIn"
QT_MOC_LITERAL(12, 114, 7) // "zoomOut"

    },
    "CanvasBase\0sigCurShapeChanged\0\0shapeName\0"
    "sigAddShape\0sigAddPoint\0point\0"
    "sigSetProperty\0ShapePtr\0shape\0clean\0"
    "zoomIn\0zoomOut"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CanvasBase[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    0,   52,    2, 0x06 /* Public */,
       5,    1,   53,    2, 0x06 /* Public */,
       7,    1,   56,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   59,    2, 0x0a /* Public */,
      11,    0,   60,    2, 0x0a /* Public */,
      12,    0,   61,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPointF,    6,
    QMetaType::Void, 0x80000000 | 8,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CanvasBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CanvasBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigCurShapeChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->sigAddShape(); break;
        case 2: _t->sigAddPoint((*reinterpret_cast< const QPointF(*)>(_a[1]))); break;
        case 3: _t->sigSetProperty((*reinterpret_cast< ShapePtr(*)>(_a[1]))); break;
        case 4: _t->clean(); break;
        case 5: _t->zoomIn(); break;
        case 6: _t->zoomOut(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CanvasBase::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasBase::sigCurShapeChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CanvasBase::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasBase::sigAddShape)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CanvasBase::*)(const QPointF & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasBase::sigAddPoint)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CanvasBase::*)(ShapePtr );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasBase::sigSetProperty)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CanvasBase::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_CanvasBase.data,
    qt_meta_data_CanvasBase,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CanvasBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CanvasBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CanvasBase.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    if (!strcmp(_clname, "org.qt-project.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    return QObject::qt_metacast(_clname);
}

int CanvasBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CanvasBase::sigCurShapeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CanvasBase::sigAddShape()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CanvasBase::sigAddPoint(const QPointF & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CanvasBase::sigSetProperty(ShapePtr _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
