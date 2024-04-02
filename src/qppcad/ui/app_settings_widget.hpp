#ifndef QPPCAD_APP_SETTINGS_WIDGET_H
#define QPPCAD_APP_SETTINGS_WIDGET_H
#include <qppcad/core/qppcad.hpp>
#include <QWidget>
#include <QSplitter>
#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <qppcad/ui/qspoiler_widget.hpp>
#include <qppcad/render/camera.hpp>

namespace qpp {

  namespace cad {

    class app_settings_widget_t : public QDialog {
        Q_OBJECT
      public:
        explicit app_settings_widget_t(QWidget *parent = 0);
        QVBoxLayout *settings_layout;
        QListWidget *settings_entries;
        QHBoxLayout *settings_splitter_layout;

        QHBoxLayout *settings_button_layout;
        QPushButton *settings_close;
        QPushButton *settings_save;

        QStackedWidget *settings_stacked_pages;
        QWidget *settings_page_general;
        QWidget *settings_page_appearance;
        QWidget *settings_page_mouse;
        QWidget *settings_page_console;
        QWidget *settings_page_ptable;

      static constexpr int sldmax = 200;
      
      QSlider * sld_mouse_svty_rot;
      QSlider * sld_mouse_svty_transl;
      QSlider * sld_mouse_wheel_step;
      QDoubleSpinBox * sb_mouse_wheel_step;
      float new_mouse_svty_rot;
      float new_mouse_svty_transl;
      float new_mouse_wheel_step;

      qspoiler_widget_t* mouse_settings();

    public slots:
      
      void save_button_clicked(); 
      void cancel_button_clicked(); 

      void mouse_svty_rot_valueChanged();
      void mouse_svty_transl_valueChanged();
      void mouse_wheel_step_sld_valueChanged();
      void mouse_wheel_step_sb_valueChanged(double d);
    };

  } // namespace qpp::cad

} // namespace qpp

#endif
