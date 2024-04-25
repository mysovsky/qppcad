#include <qppcad/ui/create_arrow_array.hpp>
#include <qppcad/core/app_state.hpp>

#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/ws_item/geom_view/geom_view_anim_subsys.hpp>
#include <qppcad/ws_item/psg_view/psg_view.hpp>
#include <qppcad/ws_item/pgf_producer/pgf_producer.hpp>
#include <qppcad/ws_item/cube_primitive/cube_primitive.hpp>
#include <qppcad/ws_item/arrow_primitive/arrow_primitive.hpp>
#include <qppcad/ws_item/node_book/node_book.hpp>
#include <qppcad/ws_item/arrow_array/arrow_array.hpp>
#include <qppcad/ws_item/py_note_book/py_note_book.hpp>

#include <qppcad/ui/qt_helpers.hpp>

#include <symm/shoenflis.hpp>
#include <vector>

using namespace qpp;
using namespace qpp::cad;

const int label_width = 90;

create_arrow_array_widget_t::create_arrow_array_widget_t() {

  bool gv_selected = false;
  anim_exists = false;
  app_state_t *astate = app_state_t::get_inst();
  auto cur_ws = astate->ws_mgr->get_cur_ws();
  if (cur_ws){
    auto cur_item = cur_ws -> get_selected_sp();
    if (cur_item){
      auto cur_gv = cur_item -> cast_as<geom_view_t>();
      if (cur_gv) {
	gv_selected = true;
	geom = cur_gv -> m_geom;
	auto anim = cur_gv -> m_anim;
	if (anim && anim->get_current_anim()){

	  frame_count = anim -> current_frame_count();
	  cur_frame = anim -> current_frame_truncated();
	  anim_exists = true;

	  //auto cur_anim = anim->get_current_anim();
	  astate->log(fmt::format("anim= {} total = {} fr tot= {} fr cur = {}\n",
	  			  anim, anim->get_total_anims(), frame_count, cur_frame));
}
      }
    }
  }
  if (!gv_selected){
    int ret = QMessageBox::warning(this,
				   tr("Geometry is not selected"),
				   tr("Arrow array can be created for certain geometry. Please select a geometry in Object Inspector first!"));
    QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
  }
  
  setWindowTitle(tr("Create new arrow array (vector field)"));
  //setFixedWidth(650);
  //setFixedHeight(500);

  main_lt = new QVBoxLayout;
  data_lt = new QHBoxLayout;
  button_lt = new QHBoxLayout;
  sub_data_lt = new QVBoxLayout;

  ok_button = new QPushButton(tr("Ok"));
  ok_button->setFixedWidth(astate->size_guide.common_button_fixed_w());
  cancel_button = new QPushButton(tr("Cancel"));
  cancel_button->setFixedWidth(astate->size_guide.common_button_fixed_w());

  button_lt->addStretch();
  button_lt->addWidget(ok_button, 0, Qt::AlignCenter);
  button_lt->addWidget(cancel_button, 0, Qt::AlignCenter);
  button_lt->addStretch();

  gb_type_descr = new qspoiler_widget_t(tr("Type description"), nullptr, false);
  //gb_type_descr->setMinimumWidth(240);
  //gb_type_descr->setMaximumHeight(130);
  gb_type_descr_lt = new QVBoxLayout;
  gb_type_descr->add_content_layout(gb_type_descr_lt);

  type_descr_lbl = new QLabel;
  type_descr_lbl->setObjectName("create_arrow_array_view_type_desc");
  type_descr_lbl->setWordWrap(true);
  gb_type_descr_lt->addWidget(type_descr_lbl);
  gb_type_descr_lt->addStretch();

  gb_ctor = new qspoiler_widget_t(tr("The source of vectors:"), nullptr, false);
  //gb_ctor->setMinimumWidth(240);
  gb_ctor_lt = new QVBoxLayout;
  gb_ctor->add_content_layout(gb_ctor_lt);
  
  rb_zero =  new QRadioButton(tr("zero vectors"));
  rb_field = new QRadioButton(tr("xgeometry fields"));
  rb_frame = new QRadioButton(tr("geometry frames"));

  rb_frame -> setEnabled(anim_exists);
  
  gb_ctor_lt->addWidget(rb_zero);
  gb_ctor_lt->addWidget(rb_field);
  gb_ctor_lt->addWidget(rb_frame);

  auto label_setup = [](QLabel *label, bool visible = false) {
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setMinimumWidth(label_width);
    label->setMinimumHeight(28);
    label->setVisible(visible);
  };

  zero_descr = new qspoiler_widget_t(tr("All vectors will be zero"),nullptr,false);
  
  field_selector = new qspoiler_widget_t(tr("Select xgeometry fields:"),nullptr,false);
  auto field_selector_lt = new QVBoxLayout();

  std::vector<QLabel*> fs_lbl = {new QLabel(tr("vx:")), new QLabel(tr("vy:")), new QLabel(tr("vz:"))};
  fs_combo = {new QComboBox, new QComboBox, new QComboBox};

  std::vector<std::string> sxfields;
  for (int i=0; i<geom->nfields(); i++)
    if (geom -> field_type(i) == type_real)
      ix_fields.push_back(i);

  for (int i : ix_fields)
    sxfields.push_back( t2s(i) + ". " + geom -> field_name(i) );
  
  for (int i=0; i<3; i++){
    fs_lbl[i]->setAlignment(Qt::AlignCenter);
    auto hlt  = new QHBoxLayout();
    hlt -> addWidget(fs_lbl[i],1);
    hlt -> addWidget(fs_combo[i],3);
    for (auto s : sxfields)
      fs_combo[i] -> addItem(tr(s.c_str()));
    field_selector_lt -> addLayout(hlt);
  }
  
  field_selector -> add_content_layout(field_selector_lt);

  


  
  frame_selector = new qspoiler_widget_t(tr("Select frames:"),nullptr,false);
  auto frame_selector_lt = new QVBoxLayout();

  rb_frame_firstlast = new QRadioButton(tr("First and last frames"));
  rb_frame_currnext  = new QRadioButton(tr("Current and next frames"));
  rb_frame_numbers   = new QRadioButton(tr("Frames number:"));

  sb_frame1 = new QSpinBox();
  sb_frame1 -> setMaximum(frame_count - 1);
  sb_frame1 -> setButtonSymbols(QAbstractSpinBox::NoButtons);
  sb_frame2 = new QSpinBox();
  sb_frame2 -> setMaximum(frame_count - 1);
  sb_frame2 -> setButtonSymbols(QAbstractSpinBox::NoButtons);
  auto frames_lt = new QHBoxLayout();
  frames_lt -> addWidget(sb_frame1);
  frames_lt -> addWidget(sb_frame2);
  
  frame_selector_lt -> addWidget(rb_frame_firstlast);
  frame_selector_lt -> addWidget(rb_frame_currnext);
  frame_selector_lt -> addWidget(rb_frame_numbers);
  frame_selector_lt -> addLayout(frames_lt);
  frame_selector -> add_content_layout(frame_selector_lt);

  gb_type_param = new qspoiler_widget_t(tr("Type parameters"), nullptr, false);
  //gb_type_param->setMinimumWidth(360);
  gb_type_param_lt = new QFormLayout;
  gb_type_param->add_content_layout(gb_type_param_lt);

  type_param_name = new QLineEdit(this);
  type_param_name->setFixedWidth(astate->size_guide.obj_insp_combo_max_w());
  type_param_name_lbl = new QLabel("Name", this);
  label_setup(type_param_name_lbl, true);

  gb_type_param_lt->addRow(type_param_name_lbl, type_param_name);

  //auto cur_ws = astate->ws_mgr->get_cur_ws();
  //if (cur_ws) type_param_name->setText(tr("new_item_%1").arg(cur_ws->m_ws_items.size()));

  main_lt->addLayout(data_lt);
  data_lt->addWidget(gb_ctor);
  data_lt->addLayout(sub_data_lt);
  sub_data_lt->addWidget(zero_descr);
  sub_data_lt->addWidget(field_selector);
  sub_data_lt->addWidget(frame_selector);
  sub_data_lt->addWidget(gb_type_param);
  main_lt->addStretch();
  main_lt->addLayout(button_lt);

  setLayout(main_lt);

  connect(ok_button, &QPushButton::clicked,
          this, &create_arrow_array_widget_t::ok_button_clicked);

  connect(cancel_button, &QPushButton::clicked,
          this, &create_arrow_array_widget_t::cancel_button_clicked);
  
  
  connect(rb_zero,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_gb_zero_checked);

  connect(rb_field,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_gb_field_checked);

  connect(rb_frame,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_gb_frame_checked);

  connect(rb_frame_firstlast,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_frame_rbs_checked);
  
  connect(rb_frame_currnext,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_frame_rbs_checked);
  
  connect(rb_frame_numbers,
          &QRadioButton::toggled,
          this,
          &create_arrow_array_widget_t::react_frame_rbs_checked);
  
  rb_frame->setChecked(true);
  rb_frame_firstlast -> setChecked(true);

}

void create_arrow_array_widget_t::set_cell_ctors_visibility(bool show) {

  /*
  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_cubic_size_lbl, type_cell_ctor_cubic_size);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_ortho_lbl, type_cell_ctor_ortho);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_l3_lbl, type_cell_ctor_l3);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_a3_lbl, type_cell_ctor_a3);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_a_lbl, type_cell_ctor_a);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_b_lbl, type_cell_ctor_b);

  qt_hlp::form_lt_ctrl_visibility(show, gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_c_lbl, type_cell_ctor_c);
  */
}

void create_arrow_array_widget_t::control_top_type_parameters_visibility() {

  //hide all
  set_cell_ctors_visibility(false);
  /*
  qt_hlp::form_lt_ctrl_visibility(false,
                                      gb_type_param_lt,
                                      type_param_name,
                                      type_param_ag_lbl, type_param_ag);

  qt_hlp::form_lt_ctrl_visibility(rb_ctor_geom3d->isChecked(),
                                      gb_type_param_lt,
                                      type_param_name,
                                      type_cell_ctor_mode_lbl, type_cell_ctor_mode);

  qt_hlp::form_lt_ctrl_visibility(rb_ctor_psg->isChecked(),
                                      gb_type_param_lt,
                                      type_param_name,
                                      type_param_ag_lbl, type_param_ag);
  */

}

void create_arrow_array_widget_t::ok_button_clicked() {

  if (type_param_name->text() == "") {
      int ret = QMessageBox::warning(this,
                                     tr("Missing name!"),
                                     tr("Name is empty! Do something!"));
    } else {

      app_state_t *astate = app_state_t::get_inst();

      auto cur_ws = astate->ws_mgr->get_cur_ws();
      if (!cur_ws) return;
      auto nt =
	astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(arrow_array_view_t::get_type_static());
      auto aa_ap = nt->cast_as<arrow_array_view_t>();
      if (!aa_ap) return;
      aa_ap->m_name = type_param_name->text().toStdString();
      cur_ws->add_item_to_ws(nt);

      auto cur_it = cur_ws->get_selected_sp();
      if (cur_it && cur_it->get_type() == geom_view_t::get_type_static()) {
	aa_ap->m_src = cur_it;
	aa_ap->updated_externally(ws_item_updf_generic);
      }

      if (rb_zero -> isChecked()){
	aa_ap -> create_zero_vectors();
      }
      
      if (rb_frame -> isChecked()){
	int f1=0, f2=0;
	if (rb_frame_firstlast -> isChecked() ) {
	  f1 = 0; f2 = frame_count - 1;
	}
	if (rb_frame_currnext -> isChecked() ) {
	  f1 = cur_frame; f2 = f1+1;
	  if (f2 >= frame_count ) f2 = 0;
	}
	if (rb_frame_numbers -> isChecked() ) {
	  f1 = sb_frame1 -> value();
	  f2 = sb_frame2 -> value();
	}
	astate -> log(fmt::format("frame1 = {} frame2 = {}",f1,f2));
	aa_ap -> create_vectors_from_frames(f1,f2);
      }

      if (rb_field -> isChecked()){
	int
	  f1 = ix_fields[fs_combo[0] -> currentIndex()],
	  f2 = ix_fields[fs_combo[1] -> currentIndex()],
	  f3 = ix_fields[fs_combo[2] -> currentIndex()];
	aa_ap -> create_vectors_from_fields(f1,f2,f3);
      }

      /*
      if (rb_ctor_geom0d->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(geom_view_t::get_type_static());
          auto nt_gv = nt->cast_as<geom_view_t>();
          if (!nt_gv) return;
          nt_gv->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_geom3d->isChecked()) {

      }

      if (rb_ctor_psg->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(psg_view_t::get_type_static());
          auto nt_psg = nt->cast_as<psg_view_t>();
          if (!nt_psg) return;
          auto ag = shnfl<float>::group(type_param_ag->currentText().toStdString());
          nt_psg->m_ag =
              std::make_shared<array_group<matrix3<float>>>(ag);
          nt_psg->m_name = type_param_name->text().toStdString();
          nt_psg->update_view();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_pgf_prod->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(pgf_producer_t::get_type_static());
          auto nt_pgfp = nt->cast_as<pgf_producer_t>();
          if (!nt_pgfp) return;
          nt_pgfp->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_cube->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(cube_primitive_t::get_type_static());
          auto nt_cp = nt->cast_as<cube_primitive_t>();
          if (!nt_cp) return;
          nt_cp->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_arrow->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(arrow_primitive_t::get_type_static());
          auto nt_ap = nt->cast_as<arrow_primitive_t>();
          if (!nt_ap) return;
          nt_ap->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_node_book->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(node_book_t::get_type_static());
          auto nb_ap = nt->cast_as<node_book_t>();
          if (!nb_ap) return;
          nb_ap->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);
        }

      if (rb_ctor_arrow_array->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(arrow_array_view_t::get_type_static());
          auto aa_ap = nt->cast_as<arrow_array_view_t>();
          if (!aa_ap) return;
          aa_ap->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);

          auto cur_it = cur_ws->get_selected_sp();
          if (cur_it && cur_it->get_type() == geom_view_t::get_type_static()) {
              aa_ap->m_src = cur_it;
              aa_ap->updated_externally(ws_item_updf_generic);
            }
        }

      if (rb_ctor_py_note_book->isChecked()) {
          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(py_note_book_t::get_type_static());
          auto aa_ap = nt->cast_as<py_note_book_t>();
          if (!aa_ap) return;
          aa_ap->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);

        }
      */
      accept();
      
  }

}

void create_arrow_array_widget_t::cancel_button_clicked() {

  reject();

}

void create_arrow_array_widget_t::react_gb_zero_checked(bool checked) {
  control_top_type_parameters_visibility();
  if (checked) {
    zero_descr -> setVisible(false);
    field_selector -> setVisible(false);
    frame_selector -> setVisible(false);
    zero_descr -> setVisible(true);
  }
}

void create_arrow_array_widget_t::react_gb_field_checked(bool checked) {
  control_top_type_parameters_visibility();
  if (checked) {
    zero_descr -> setVisible(false);
    field_selector -> setVisible(false);
    frame_selector -> setVisible(false);
    field_selector -> setVisible(true);
  }
}

void create_arrow_array_widget_t::react_gb_frame_checked(bool checked) {
  control_top_type_parameters_visibility();
  if (checked) {
    zero_descr -> setVisible(false);
    field_selector -> setVisible(false);
    frame_selector -> setVisible(true);
  }
}

void create_arrow_array_widget_t::react_frame_rbs_checked(bool checked) {
  control_top_type_parameters_visibility();
  sb_frame1 -> setEnabled(rb_frame_numbers->isChecked());
  sb_frame2 -> setEnabled(rb_frame_numbers->isChecked());
}



