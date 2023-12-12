/****************************************************************************
** Meta object code from reading C++ file 'canvasbase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/canvas/canvasbase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'canvasbase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CanvasBase_t {
    QByteArrayData data[17];
    char stringdata0[155];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CanvasBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CanvasBase_t qt_meta_stringdata_CanvasBase = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CanvasBase"
QT_MOC_LITERAL(1, 11, 11), // "modeChanged"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 4), // "mode"
QT_MOC_LITERAL(4, 29, 8), // "setScale"
QT_MOC_LITERAL(5, 38, 8), // "newScale"
QT_MOC_LITERAL(6, 47, 11), // "setPenWidth"
QT_MOC_LITERAL(7, 59, 10), // "changeTask"
QT_MOC_LITERAL(8, 70, 8), // "TaskMode"
QT_MOC_LITERAL(9, 79, 5), // "_task"
QT_MOC_LITERAL(10, 85, 16), // "changeCanvasMode"
QT_MOC_LITERAL(11, 102, 10), // "CanvasMode"
QT_MOC_LITERAL(12, 113, 5), // "_mode"
QT_MOC_LITERAL(13, 119, 14), // "changeDrawMode"
QT_MOC_LITERAL(14, 134, 8), // "DrawMode"
QT_MOC_LITERAL(15, 143, 5), // "_draw"
QT_MOC_LITERAL(16, 149, 5) // "close"

    },
    "CanvasBase\0modeChanged\0\0mode\0setScale\0"
    "newScale\0setPenWidth\0changeTask\0"
    "TaskMode\0_task\0changeCanvasMode\0"
    "CanvasMode\0_mode\0changeDrawMode\0"
    "DrawMode\0_draw\0close"
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
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   52,    2, 0x0a /* Public */,
       6,    1,   55,    2, 0x0a /* Public */,
       7,    1,   58,    2, 0x0a /* Public */,
      10,    1,   61,    2, 0x0a /* Public */,
      13,    1,   64,    2, 0x0a /* Public */,
      16,    0,   67,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,    5,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void,

       0        // eod
};

void CanvasBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CanvasBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->modeChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->setScale((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 2: _t->setPenWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->changeTask((*reinterpret_cast< TaskMode(*)>(_a[1]))); break;
        case 4: _t->changeCanvasMode((*reinterpret_cast< CanvasMode(*)>(_a[1]))); break;
        case 5: _t->changeDrawMode((*reinterpret_cast< DrawMode(*)>(_a[1]))); break;
        case 6: _t->close(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CanvasBase::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CanvasBase::modeChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CanvasBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
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
    return QWidget::qt_metacast(_clname);
}

int CanvasBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void CanvasBase::modeChanged(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
