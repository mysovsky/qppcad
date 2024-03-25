#ifndef QPPCAD_CREATE_ARROW_ARRAY_WIDGET
#define QPPCAD_CREATE_ARROW_ARRAY_WIDGET

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

        QVBoxLayout *main_lt;
        QHBoxLayout *data_lt;
        QVBoxLayout *sub_data_lt;
        QHBoxLayout *button_lt;
        QPushButton *ok_button;
        QPushButton *cancel_button;

        qspoiler_widget_t *gb_ctor{nullptr};
        QVBoxLayout *gb_ctor_lt{nullptr};
       
        QRadioButton *rb_zero{nullptr};
        QRadioButton *rb_field{nullptr};
        QRadioButton *rb_frame{nullptr};

        qspoiler_widget_t * zero_descr, * field_selector, * frame_selector;

        QRadioButton *rb_frame_firstlast, *rb_frame_currnext, *rb_frame_numbers;
        QSpinBox *sb_frame1, *sb_frame2;
	
        qspoiler_widget_t *gb_type_descr{nullptr};
        QVBoxLayout *gb_type_descr_lt{nullptr};
        QLabel *type_descr_lbl{nullptr};
      
        qspoiler_widget_t *gb_type_param{nullptr};
        QFormLayout *gb_type_param_lt{nullptr};

        QLabel *type_param_name_lbl{nullptr};
        QLineEdit *type_param_name{nullptr};
      
        int frame_count{0}, cur_frame{0};
        bool anim_exists{false};
      
        create_arrow_array_widget_t();

        void set_cell_ctors_visibility(bool show);
        void control_top_type_parameters_visibility();

      public slots:

        void ok_button_clicked();
        void cancel_button_clicked();

        void react_gb_zero_checked(bool checked);
        void react_gb_field_checked(bool checked);
        void react_gb_frame_checked(bool checked);      

        void react_frame_rbs_checked(bool checked);      

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
