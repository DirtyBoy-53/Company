#include "ImageAdj.h"

#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include "confile.h"
#include "ydefine.h"

ImageAdj::ImageAdj(QWidget* parent)
	: QWidget(parent)
{
	setWindowIcon(QIcon(":ICON"));
	initParam();
	initUI();
	initConnect();
}

void ImageAdj::setParam(const int& contrast, const int& brightness)
{
	m_contrast = contrast;
	m_brightness = brightness;
	m_contrastSlider->setValue(m_contrast);
	m_brightnessSlider->setValue(m_brightness);
}

ImageAdj::~ImageAdj()
{}

void ImageAdj::initParam()
{
	m_maxContrast	= g_config->Get<int>("max_contrast",	"image", MAX_CONTRAST);
	m_minContrast	= g_config->Get<int>("min_contrast",	"image", MIN_CONTRAST);
	m_maxBrightness = g_config->Get<int>("max_brightness",	"image", MAX_BRIGHTNESS);
	m_minBrightness = g_config->Get<int>("min_brightness",	"image", MIN_BRIGHTNESS);
}

void ImageAdj::initUI()
{
	auto bLayout1 = new QHBoxLayout();
	m_contrastSlider   = createSlider(bLayout1, "对比度:", m_minContrast, m_maxContrast, m_contrast);
	auto bLayout2 = new QHBoxLayout();
	m_brightnessSlider = createSlider(bLayout2, "亮  度:", m_minBrightness, m_maxBrightness, m_brightness);

	auto vLayout = new QVBoxLayout();
	vLayout->addLayout(bLayout1);
	vLayout->addLayout(bLayout2);
	setLayout(vLayout);
}

void ImageAdj::initConnect()
{
	connect(m_contrastSlider, &QSlider::valueChanged, this, [this](int value) {
		m_contrast = value;
		emit contrastChanged(m_contrast); 
		g_config->Set<int>("default_contrast", m_contrast, "image");
	});
	connect(m_brightnessSlider, &QSlider::valueChanged, this, [this](int value) {
		m_brightness = value;
		emit brightnessChanged(m_brightness);
		g_config->Set<int>("default_brightness", m_brightness, "image");
	});
}

QSlider* ImageAdj::createSlider(QHBoxLayout* layout, 
								const QString& title, 
								int min, int max, int value)
{
	auto *label = new QLabel(title);
	auto slider = new QSlider(Qt::Horizontal);
	layout->addWidget(label);
	layout->addWidget(slider);
	slider->setMinimum(min);
	slider->setMaximum(max);
	slider->setValue(value);
	return slider;
}
