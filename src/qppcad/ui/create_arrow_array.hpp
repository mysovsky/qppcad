#ifndef QPPCAD_CREATE_ARROW_ARRAY
#define QPPCAD_CREATE_ARROW_ARRAY

#include <qppcad/core/qppcad.hpp>
#include <qppcad/ui/qbinded_inputs.hpp>
#include <qppcad/ui/qspoiler_widget.hpp>

#include <QWidget>
#include <QDialog>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QApplication>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QDoubleSpinBox>

namespace qpp {

  namespace cad {
    
    class create_arrow_array_widget_t : public QDialog {

    public:
      
        QPushButton *ok_button;
        QPushButton *cancel_button;
        QVBoxLayout *main_lt;
        QHBoxLayout *data_lt;
        QVBoxLayout *sub_data_lt;
        QHBoxLayout *button_lt;
            
        create_arrow_array_widget_t();

      public slots:

        void ok_button_clicked();
        void cancel_button_clicked();      
    };
    
  } //namespace cad

} //namespace qpp

#endif
