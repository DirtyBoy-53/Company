/****************************************************************************
** Meta object code from reading C++ file 'canvas2d.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/canvas/canvas2d.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'canvas2d.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Canvas2D_t {
    QByteArrayData data[27];
    char stringdata0[287];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Canvas2D_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Canvas2D_t qt_meta_stringdata_Canvas2D = {
    {
QT_MOC_LITERAL(0, 0, 8), // "Canvas2D"
QT_MOC_LITERAL(1, 9, 10), // "mouseMoved"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 3), // "pos"
QT_MOC_LITERAL(4, 25, 21), // "newRectangleAnnotated"
QT_MOC_LITERAL(5, 47, 7), // "newRect"
QT_MOC_LITERAL(6, 55, 19), // "newStrokesAnnotated"
QT_MOC_LITERAL(7, 75, 16), // "QList<SegStroke>"
QT_MOC_LITERAL(8, 92, 7), // "strokes"
QT_MOC_LITERAL(9, 100, 25), // "modifySelectedRectRequest"
QT_MOC_LITERAL(10, 126, 3), // "idx"
QT_MOC_LITERAL(11, 130, 4), // "rect"
QT_MOC_LITERAL(12, 135, 17), // "removeRectRequest"
QT_MOC_LITERAL(13, 153, 8), // "setScale"
QT_MOC_LITERAL(14, 162, 11), // "setPenWidth"
QT_MOC_LITERAL(15, 174, 5), // "width"
QT_MOC_LITERAL(16, 180, 10), // "changeTask"
QT_MOC_LITERAL(17, 191, 8), // "TaskMode"
QT_MOC_LITERAL(18, 200, 5), // "_task"
QT_MOC_LITERAL(19, 206, 16), // "changeCanvasMode"
QT_MOC_LITERAL(20, 223, 10), // "CanvasMode"
QT_MOC_LITERAL(21, 234, 5), // "_mode"
QT_MOC_LITERAL(22, 240, 14), // "changeDrawMode"
QT_MOC_LITERAL(23, 255, 8), // "DrawMode"
QT_MOC_LITERAL(24, 264, 5), // "_draw"
QT_MOC_LITERAL(25, 270, 5), // "close"
QT_MOC_LITERAL(26, 276, 10) // "loadPixmap"

    },
    "Canvas2D\0mouseMoved\0\0pos\0newRectangleAnnotated\0"
    "newRect\0newStrokesAnnotated\0"
    "QList<SegStroke>\0strokes\0"
    "modifySelectedRectRequest\0idx\0rect\0"
    "removeRectRequest\0setScale\0setPenWidth\0"
    "width\0changeTask\0TaskMode\0_task\0"
    "changeCanvasMode\0CanvasMode\0_mode\0"
    "changeDrawMode\0DrawMode\0_draw\0close\0"
    "loadPixmap"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Canvas2D[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       4,    1,   77,    2, 0x06 /* Public */,
       6,    1,   80,    2, 0x06 /* Public */,
       9,    2,   83,    2, 0x06 /* Public */,
      12,    1,   88,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    1,   91,    2, 0x0a /* Public */,
      14,    1,   94,    2, 0x0a /* Public */,
      16,    1,   97,    2, 0x0a /* Public */,
      19,    1,  100,    2, 0x0a /* Public */,
      22,    1,  103,    2, 0x0a /* Public */,
      25,    0,  106,    2, 0x0a /* Public */,
      26,    1,  107,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPointF,    3,
    QMetaType::Void, QMetaType::QRect,    5,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::QRect,   10,   11,
    QMetaType::Void, QMetaType::Int,   10,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,    2,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 23,   24,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPixmap,    2,

       0        // eod
};

void Canvas2D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Canvas2D *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->mouseMoved((*reinterpret_cast< QPointF(*)>(_a[1]))); break;
        case 1: _t->newRectangleAnnotated((*reinterpret_cast< QRect(*)>(_a[1]))); break;
        case 2: _t->newStrokesAnnotated((*reinterpret_cast< const QList<SegStroke>(*)>(_a[1]))); break;
        case 3: _t->modifySelectedRectRequest((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QRect(*)>(_a[2]))); break;
        case 4: _t->removeRectRequest((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->setScale((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 6: _t->setPenWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->changeTask((*reinterpret_cast< TaskMode(*)>(_a[1]))); break;
        case 8: _t->changeCanvasMode((*reinterpret_cast< CanvasMode(*)>(_a[1]))); break;
        case 9: _t->changeDrawMode((*reinterpret_cast< DrawMode(*)>(_a[1]))); break;
        case 10: _t->close(); break;
        case 11: _t->loadPixmap((*reinterpret_cast< QPixmap(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Canvas2D::*)(QPointF );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Canvas2D::mouseMoved)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Canvas2D::*)(QRect );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Canvas2D::newRectangleAnnotated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Canvas2D::*)(const QList<SegStroke> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Canvas2D::newStrokesAnnotated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Canvas2D::*)(int , QRect );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Canvas2D::modifySelectedRectRequest)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Canvas2D::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Canvas2D::removeRectRequest)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Canvas2D::staticMetaObject = { {
    &CanvasBase::staticMetaObject,
    qt_meta_stringdata_Canvas2D.data,
    qt_meta_data_Canvas2D,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Canvas2D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Canvas2D::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Canvas2D.stringdata0))
        return static_cast<void*>(this);
    return CanvasBase::qt_metacast(_clname);
}

int Canvas2D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CanvasBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void Canvas2D::mouseMoved(QPointF _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Canvas2D::newRectangleAnnotated(QRect _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Canvas2D::newStrokesAnnotated(const QList<SegStroke> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Canvas2D::modifySelectedRectRequest(int _t1, QRect _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Canvas2D::removeRectRequest(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
