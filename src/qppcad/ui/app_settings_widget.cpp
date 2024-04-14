#include <qppcad/ui/app_settings_widget.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

app_settings_widget_t::app_settings_widget_t(QWidget *parent) : QDialog (parent) {

  setWindowTitle(tr("Application settings"));
  setMinimumWidth(350);
  settings_layout = new QVBoxLayout;
  settings_splitter_layout = new QHBoxLayout;

  settings_entries = new QListWidget;
  settings_entries->addItem(tr("General"));
  settings_entries->addItem(tr("Appearance"));
  settings_entries->addItem(tr("Mouse"));
  settings_entries->addItem(tr("Console"));
  settings_entries->addItem(tr("Periodic table"));
  settings_entries->setMaximumWidth(150);

  settings_stacked_pages = new QStackedWidget;
  settings_stacked_pages->setMinimumWidth(600);
  settings_stacked_pages->setMinimumHeight(500);

  settings_page_general = new qspoiler_widget_t(tr("General settings"),this, false);
  //settings_page_general->setMinimumWidth(200);

  settings_page_appearance = new QWidget;
  settings_page_appearance->setMinimumWidth(200);
  
  settings_page_mouse = mouse_settings();
  //settings_page_mouse -> setMinimumWidth(200);

  settings_page_console = new QWidget;
  settings_page_console->setMinimumWidth(200);

  settings_page_ptable = new QWidget;
  settings_page_ptable->setMinimumWidth(200);

  settings_stacked_pages->addWidget(settings_page_general);
  settings_stacked_pages->addWidget(settings_page_appearance);
  settings_stacked_pages->addWidget(settings_page_mouse);
  settings_stacked_pages->addWidget(settings_page_console);
  settings_stacked_pages->addWidget(settings_page_ptable);

  settings_splitter_layout = new QHBoxLayout;
  setLayout(settings_layout);
  settings_layout->addLayout(settings_splitter_layout);
  settings_splitter_layout->addWidget(settings_entries);
  settings_splitter_layout->addWidget(settings_stacked_pages);

  connect(settings_entries,
          &QListWidget::currentRowChanged,
          settings_stacked_pages,
          &QStackedWidget::setCurrentIndex);

  settings_entries->setCurrentRow(0);

  settings_close = new QPushButton(tr("Close"));
  settings_close->setMaximumWidth(75);

  settings_save = new QPushButton(tr("Save"));
  settings_save->setMaximumWidth(75);

  settings_button_layout = new QHBoxLayout;
  settings_button_layout->setAlignment(Qt::AlignRight);
  settings_button_layout->addWidget(settings_close, 0, Qt::AlignRight);
  settings_button_layout->addWidget(settings_save, 0, Qt::AlignRight);
  settings_layout->addLayout(settings_button_layout);

  connect(settings_close,
          &QPushButton::clicked,
          this,
          &app_settings_widget_t::cancel_button_clicked);

  connect(settings_save,
          &QPushButton::clicked,
          this,
          &app_settings_widget_t::save_button_clicked);

}

int iround(float f){
  return int(f + .5 ) - (f<0);
}

float slider_to_float(int v, float x0, float x1){
  return x0 + (x1-x0)*v/app_settings_widget_t::sldmax;
}

int float_to_slider(float v, float x0, float x1){
  return iround(app_settings_widget_t::sldmax*(v - x0) /(x1-x0));
}


qspoiler_widget_t* app_settings_widget_t::mouse_settings(){
  auto q = new qspoiler_widget_t(tr("Mouse settings"),this, false, 6, 1200);
  
  setStyleSheet("QSlider::groove:horizontal {"
		"height: 6px;"
		"margin: 8px;"
		"border: 1px solid #c17d08;"
		"border-radius: 12px;"
		"}"
		"QSlider::handle:horizontal {"
		"border: 1px solid #777;"
		"width: 13px;"
		"margin-top: -6px;"
		"margin-bottom: -6px;"
		"border-radius: 4px;"
		"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"stop:0 #eee, stop:1 #ccc);"
		"border: 1px solid #777;"
		"}"
		);
  //auto bg_clr = QWidget::palette().color(QWidget::backgroundRole());
  //auto fg_clr = QWidget::palette().color(QWidget::foregroundRole());
  
  auto lt = new QVBoxLayout;
  q -> add_content_layout(lt);

  lt -> addWidget(new QLabel(tr("Mouse sensitivity for rotation"), this));
  sld_mouse_senty_rot = new QSlider(Qt::Horizontal, this);
  lt -> addWidget(sld_mouse_senty_rot);
  sld_mouse_senty_rot -> setMinimum(1);
  sld_mouse_senty_rot -> setMaximum(sldmax);
  new_mouse_senty_rot = camera_t::mouse_senty_rot;
  sld_mouse_senty_rot -> setValue(float_to_slider(new_mouse_senty_rot, 0e0, camera_t::max_mouse_senty));
  QObject::connect(sld_mouse_senty_rot, & QSlider::valueChanged,
		   this, & app_settings_widget_t::mouse_senty_rot_valueChanged);
  
  lt -> addWidget(new QLabel(tr("Mouse sensitivity for translation"), this));
  sld_mouse_senty_transl = new QSlider(Qt::Horizontal, this);
  lt -> addWidget(sld_mouse_senty_transl);
  sld_mouse_senty_transl -> setMinimum(1);
  sld_mouse_senty_transl -> setMaximum(sldmax);
  new_mouse_senty_transl = camera_t::mouse_senty_transl;
  sld_mouse_senty_transl -> setValue(float_to_slider(new_mouse_senty_transl, 0e0, camera_t::max_mouse_senty));
  QObject::connect(sld_mouse_senty_transl, & QSlider::valueChanged,
		   this, & app_settings_widget_t::mouse_senty_transl_valueChanged);
  
  sld_mouse_wheel_step = new QSlider(Qt::Horizontal, this);
  sld_mouse_wheel_step -> setMinimum(1);
  sld_mouse_wheel_step -> setMaximum(sldmax);
  new_mouse_wheel_step = camera_t::m_mouse_wheel_camera_step;
  sld_mouse_wheel_step -> setValue(float_to_slider(new_mouse_wheel_step, 0e0, camera_t::max_mouse_wheel_step));
  sb_mouse_wheel_step = new QDoubleSpinBox(this);
  sb_mouse_wheel_step -> setValue(new_mouse_wheel_step);
  sb_mouse_wheel_step -> setMinimum(0.01);
  sb_mouse_wheel_step -> setMaximum(camera_t::max_mouse_wheel_step);
  sb_mouse_wheel_step -> setSingleStep(0.1);
  auto mwlt = new QHBoxLayout;
  lt -> addLayout(mwlt);
  mwlt -> addWidget(new QLabel(tr("Mouse wheel step"), this));
  mwlt -> addWidget(sb_mouse_wheel_step);
  mwlt -> addStretch();
  lt -> addWidget(sld_mouse_wheel_step);
  QObject::connect(sld_mouse_wheel_step, & QSlider::valueChanged,
		   this, & app_settings_widget_t::mouse_wheel_step_sld_valueChanged);
  QObject::connect(sb_mouse_wheel_step,
		   static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		   this, & app_settings_widget_t::mouse_wheel_step_sb_valueChanged);

  lt -> addStretch();
  
  return q;
}

void app_settings_widget_t::mouse_senty_rot_valueChanged(){
  new_mouse_senty_rot = slider_to_float(sld_mouse_senty_rot -> value(), 0e0, camera_t::max_mouse_senty);
}

void app_settings_widget_t::mouse_senty_transl_valueChanged(){
  new_mouse_senty_transl = slider_to_float(sld_mouse_senty_transl -> value(), 0e0, camera_t::max_mouse_senty);
}

void app_settings_widget_t::mouse_wheel_step_sld_valueChanged(){
  new_mouse_wheel_step = slider_to_float(sld_mouse_wheel_step -> value(), 0e0, camera_t::max_mouse_wheel_step);
  sb_mouse_wheel_step -> setValue(new_mouse_wheel_step);
}

void app_settings_widget_t::mouse_wheel_step_sb_valueChanged(double d){
  new_mouse_wheel_step = d;
  sld_mouse_wheel_step -> setValue(float_to_slider(d, 0e0, camera_t::max_mouse_wheel_step));
}

void app_settings_widget_t::save_button_clicked() {
  
  camera_t::mouse_senty_rot = new_mouse_senty_rot;
  camera_t::mouse_senty_transl = new_mouse_senty_transl;
  camera_t::m_mouse_wheel_camera_step = new_mouse_wheel_step;
   
  accept();
}

void app_settings_widget_t::cancel_button_clicked() {
  reject();
}
