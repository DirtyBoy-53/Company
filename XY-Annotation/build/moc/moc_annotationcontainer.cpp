/****************************************************************************
** Meta object code from reading C++ file 'annotationcontainer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/controllers/annotationcontainer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'annotationcontainer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AnnotationContainer_t {
    QByteArrayData data[25];
    char stringdata0[284];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AnnotationContainer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AnnotationContainer_t qt_meta_stringdata_AnnotationContainer = {
    {
QT_MOC_LITERAL(0, 0, 19), // "AnnotationContainer"
QT_MOC_LITERAL(1, 20, 15), // "selectedChanged"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 11), // "annoChanged"
QT_MOC_LITERAL(4, 49, 13), // "labelGiveBack"
QT_MOC_LITERAL(5, 63, 5), // "label"
QT_MOC_LITERAL(6, 69, 17), // "UndoEnableChanged"
QT_MOC_LITERAL(7, 87, 17), // "RedoEnableChanged"
QT_MOC_LITERAL(8, 105, 15), // "AnnotationAdded"
QT_MOC_LITERAL(9, 121, 11), // "AnnoItemPtr"
QT_MOC_LITERAL(10, 133, 4), // "item"
QT_MOC_LITERAL(11, 138, 18), // "AnnotationInserted"
QT_MOC_LITERAL(12, 157, 3), // "idx"
QT_MOC_LITERAL(13, 161, 18), // "AnnotationModified"
QT_MOC_LITERAL(14, 180, 17), // "AnnotationRemoved"
QT_MOC_LITERAL(15, 198, 14), // "AnnotationSwap"
QT_MOC_LITERAL(16, 213, 10), // "allCleared"
QT_MOC_LITERAL(17, 224, 9), // "push_back"
QT_MOC_LITERAL(18, 234, 6), // "remove"
QT_MOC_LITERAL(19, 241, 6), // "modify"
QT_MOC_LITERAL(20, 248, 4), // "swap"
QT_MOC_LITERAL(21, 253, 8), // "allClear"
QT_MOC_LITERAL(22, 262, 4), // "redo"
QT_MOC_LITERAL(23, 267, 4), // "undo"
QT_MOC_LITERAL(24, 272, 11) // "setSelected"

    },
    "AnnotationContainer\0selectedChanged\0"
    "\0annoChanged\0labelGiveBack\0label\0"
    "UndoEnableChanged\0RedoEnableChanged\0"
    "AnnotationAdded\0AnnoItemPtr\0item\0"
    "AnnotationInserted\0idx\0AnnotationModified\0"
    "AnnotationRemoved\0AnnotationSwap\0"
    "allCleared\0push_back\0remove\0modify\0"
    "swap\0allClear\0redo\0undo\0setSelected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AnnotationContainer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  109,    2, 0x06 /* Public */,
       3,    0,  110,    2, 0x06 /* Public */,
       4,    1,  111,    2, 0x06 /* Public */,
       6,    1,  114,    2, 0x06 /* Public */,
       7,    1,  117,    2, 0x06 /* Public */,
       8,    1,  120,    2, 0x06 /* Public */,
      11,    2,  123,    2, 0x06 /* Public */,
      13,    2,  128,    2, 0x06 /* Public */,
      14,    1,  133,    2, 0x06 /* Public */,
      15,    1,  136,    2, 0x06 /* Public */,
      16,    0,  139,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      17,    1,  140,    2, 0x0a /* Public */,
      18,    1,  143,    2, 0x0a /* Public */,
      19,    2,  146,    2, 0x0a /* Public */,
      20,    1,  151,    2, 0x0a /* Public */,
      21,    0,  154,    2, 0x0a /* Public */,
      22,    0,  155,    2, 0x0a /* Public */,
      23,    0,  156,    2, 0x0a /* Public */,
      24,    1,  157,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 9, QMetaType::Int,   10,   12,
    QMetaType::Void, 0x80000000 | 9, QMetaType::Int,   10,   12,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 9,   12,   10,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,

       0        // eod
};

void AnnotationContainer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AnnotationContainer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectedChanged(); break;
        case 1: _t->annoChanged(); break;
        case 2: _t->labelGiveBack((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->UndoEnableChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->RedoEnableChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->AnnotationAdded((*reinterpret_cast< AnnoItemPtr(*)>(_a[1]))); break;
        case 6: _t->AnnotationInserted((*reinterpret_cast< AnnoItemPtr(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->AnnotationModified((*reinterpret_cast< AnnoItemPtr(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: _t->AnnotationRemoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->AnnotationSwap((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->allCleared(); break;
        case 11: _t->push_back((*reinterpret_cast< const AnnoItemPtr(*)>(_a[1]))); break;
        case 12: _t->remove((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->modify((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const AnnoItemPtr(*)>(_a[2]))); break;
        case 14: _t->swap((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->allClear(); break;
        case 16: _t->redo(); break;
        case 17: _t->undo(); break;
        case 18: _t->setSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AnnotationContainer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::selectedChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::annoChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::labelGiveBack)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::UndoEnableChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::RedoEnableChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(AnnoItemPtr );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::AnnotationAdded)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(AnnoItemPtr , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::AnnotationInserted)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(AnnoItemPtr , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::AnnotationModified)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::AnnotationRemoved)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::AnnotationSwap)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (AnnotationContainer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AnnotationContainer::allCleared)) {
                *result = 10;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AnnotationContainer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AnnotationContainer.data,
    qt_meta_data_AnnotationContainer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AnnotationContainer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AnnotationContainer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AnnotationContainer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AnnotationContainer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void AnnotationContainer::selectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void AnnotationContainer::annoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void AnnotationContainer::labelGiveBack(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AnnotationContainer::UndoEnableChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AnnotationContainer::RedoEnableChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AnnotationContainer::AnnotationAdded(AnnoItemPtr _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void AnnotationContainer::AnnotationInserted(AnnoItemPtr _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void AnnotationContainer::AnnotationModified(AnnoItemPtr _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void AnnotationContainer::AnnotationRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void AnnotationContainer::AnnotationSwap(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void AnnotationContainer::allCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
