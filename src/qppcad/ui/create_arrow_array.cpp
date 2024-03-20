#include <qppcad/ui/create_arrow_array.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

create_arrow_array_widget_t::create_arrow_array_widget_t(){

  app_state_t* astate = app_state_t::get_inst();
  
  setWindowTitle(tr("Create new arrow array"));
		    
  ok_button = new QPushButton(tr("Ok"));
  ok_button->setFixedWidth(astate->size_guide.common_button_fixed_w());
  cancel_button = new QPushButton(tr("Cancel"));
  cancel_button->setFixedWidth(astate->size_guide.common_button_fixed_w());
  
  connect(ok_button, &QPushButton::clicked,
          this, &create_arrow_array_widget_t::ok_button_clicked);

  connect(cancel_button, &QPushButton::clicked,
          this, &create_arrow_array_widget_t::cancel_button_clicked);
  
  main_lt = new QVBoxLayout;
  data_lt = new QHBoxLayout;
  auto rb_lt = new QVBoxLayout;
  button_lt = new QHBoxLayout;
  sub_data_lt = new QVBoxLayout;
  
  button_lt->addStretch();
  button_lt->addWidget(ok_button, 0, Qt::AlignCenter);
  button_lt->addWidget(cancel_button, 0, Qt::AlignCenter);
  button_lt->addStretch();

  auto rb_zero = new QRadioButton(tr("Zero vectors"));
  auto rb_field = new QRadioButton(tr("From xgeometry fields"));
  auto rb_frames = new QRadioButton(tr("From frames"));

  rb_lt->addStretch();
  rb_lt->addWidget(rb_zero);
  rb_lt->addWidget(rb_field);
  rb_lt->addWidget(rb_frames);
  rb_lt->addStretch();

  main_lt->addLayout(data_lt);
  data_lt->addLayout(rb_lt);
  data_lt->addLayout(sub_data_lt);
  main_lt->addStretch();
  main_lt->addLayout(button_lt);

  setLayout(main_lt);
  
}


void create_arrow_array_widget_t::ok_button_clicked(){
  accept();
}


void create_arrow_array_widget_t::cancel_button_clicked(){
  reject();
}
