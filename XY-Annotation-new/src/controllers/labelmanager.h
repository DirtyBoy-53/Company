#ifndef LABELMANAGER_H
#define LABELMANAGER_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

class LabelProperty
{
public:
    LabelProperty(QString label, QColor color, bool visible, int id);
    LabelProperty();
    QString m_label;
    QColor m_color;
    bool m_visible;
    int m_id;
    int m_groupId;
    QString m_description;

    void operator = (const LabelProperty &label);
    void setProperty(QString label, QColor color, bool visible, int id);

    QJsonObject toJsonObject();
    void fromJsonObject(QJsonObject json);
};

class LabelManager : public QObject
{
    Q_OBJECT
public:
    explicit LabelManager(QObject *parent = nullptr);

    LabelProperty operator[](QString label) const { return labels[label]; }
    bool hasLabel(QString label) const { return labels.find(label)!=labels.end(); }
    QList<LabelProperty> getLabels() const { return labels.values(); }
    QColor getColor(QString label) const { checkLabel(label); return labels[label].m_color; }

    QJsonArray toJsonArray();
    void fromJsonArray(QJsonArray json);

    void fromJsonObject(QJsonObject json);

signals:
    void labelChanged();
    void labelRemoved(QString label);
    void labelAdded(QString label, QColor color, bool visibile,int id);
    void visibelChanged(QString label, bool visible);
    void colorChanged(QString label, QColor color);
    void allCleared();
public slots:
    void addLabel(QString label, QColor color, bool visible, int id=-1);
    void removeLabel(QString label);
    void setColor(QString label, QColor color);
    void setVisible(QString label, bool visible);
    void allClear();

private:
    QMap<QString, LabelProperty> labels;
    void checkLabel(QString label) const;

    int currentMaxId;
};

#endif // LABELMANAGER_H
