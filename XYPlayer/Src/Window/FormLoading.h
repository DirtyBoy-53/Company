#ifndef FORMLOADING_H
#define FORMLOADING_H

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class FormLoading : public QDialog
{
    Q_OBJECT
public:
    explicit FormLoading(QWidget *parent = nullptr);
    ~FormLoading();
private:
    void initUI();
    void initConnect();
protected:
    virtual void mousePressEvent(QMouseEvent *event)            override;
    virtual void mouseMoveEvent(QMouseEvent *event)             override;
    virtual void mouseReleaseEvent(QMouseEvent *event)          override;
    virtual void showEvent(QShowEvent *)                        override;

    virtual bool eventFilter(QObject *target, QEvent *event)    override;
public slots:
    void slot_OnULoaded_open();
    void slot_OnULoaded_init();
    void slot_OnUpdateInfo(const QString &msg,int type=0);
    void slot_setVisible(bool state);

    void slot_algSdkInitStart();
    void slot_algSdkInitFinished();
    void slot_algSdkOpenStart();
    void slot_algSdkOpenFinished();
private:
    bool                m_bDrag;
    QPoint              mouseStartPoint;
    QPoint              windowTopLeftPoint;
    QTimer*             m_timer_open{nullptr};
    QTimer*             m_timer_init{nullptr};
    QLabel*             lbl_outMsg;
    QPushButton*        btn_exit;
    QPushButton*        btn_enter;

    QFutureWatcher<bool>    m_futureWatcher_open;
    QFuture<bool>*           m_future_open{nullptr};
    QFutureWatcher<bool>    m_futureWatcher_init;
    QFuture<bool>*           m_future_init{nullptr};

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
};

#endif // FORMLOADING_H
