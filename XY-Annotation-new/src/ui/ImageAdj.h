#pragma once

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)

class ImageAdj  : public QWidget
{
	Q_OBJECT
public:
	ImageAdj(QWidget *parent = nullptr);
	void setParam(const int &contrast,const int &brightness);
	~ImageAdj();

private:
	void		initParam();
	void		initUI();
	void		initConnect();

private:
	QSlider		*createSlider(QHBoxLayout *layout, const QString &title, int min, int max, int value);

	QSlider		*m_contrastSlider{nullptr};
	QSlider		*m_brightnessSlider{nullptr};

	int			m_contrast{0};
	int			m_brightness{0};
	int			m_maxContrast{0};
	int			m_maxBrightness{0};
	int			m_minContrast{0};
	int			m_minBrightness{0};

signals:
	void		contrastChanged(const int &contrast);
	void		brightnessChanged(const int &brightness);
};
