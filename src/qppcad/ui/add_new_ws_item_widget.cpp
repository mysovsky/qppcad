#include <qppcad/ui/add_new_ws_item_widget.hpp>
#include <qppcad/core/app_state.hpp>

#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/ws_item/psg_view/psg_view.hpp>
#include <qppcad/ws_item/pgf_producer/pgf_producer.hpp>
#include <qppcad/ws_item/cube_primitive/cube_primitive.hpp>
#include <qppcad/ws_item/arrow_primitive/arrow_primitive.hpp>
#include <qppcad/ws_item/node_book/node_book.hpp>
#include <qppcad/ws_item/arrow_array/arrow_array.hpp>
#include <qppcad/ws_item/py_note_book/py_note_book.hpp>

#include <qppcad/ui/qt_helpers.hpp>

#include <symm/shoenflis.hpp>

using namespace qpp;
using namespace qpp::cad;

const int label_width = 90;

add_new_ws_item_widget_t::add_new_ws_item_widget_t() {

  app_state_t *astate = app_state_t::get_inst();

  setWindowTitle(tr("Add new item to workspace"));
  setFixedWidth(650);
  setFixedHeight(500);

  descr_list.reserve(11);

  descr_list.push_back(tr("Description for geom_view_t [DIM=0D]"));
  descr_list.push_back(tr("Description for geom_view_t [DIM=1D]"));
  descr_list.push_back(tr("Description for geom_view_t [DIM=2D]"));
  descr_list.push_back(tr("Description for geom_view_t [DIM=3D]"));
  descr_list.push_back(tr("Description for psg_view_t"));
  descr_list.push_back(tr("Description for pgf_producer_t"));
  descr_list.push_back(tr("Description for cube_primitive_t"));
  descr_list.push_back(tr("Description for arrow_primitive_t"));
  descr_list.push_back(tr("Description for node_book_t"));
  descr_list.push_back(tr("Description for arrow_array_t"));
  descr_list.push_back(tr("Description for py_note_book_t"));

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
  type_descr_lbl->setObjectName("add_new_ws_item_type_desc");
  type_descr_lbl->setWordWrap(true);
  gb_type_descr_lt->addWidget(type_descr_lbl);
  gb_type_descr_lt->addStretch();

  gb_ctor = new qspoiler_widget_t(tr("Constructible types"), nullptr, false);
  //gb_ctor->setMinimumWidth(240);
  gb_ctor_lt = new QVBoxLayout;
  gb_ctor->add_content_layout(gb_ctor_lt);

  rb_ctor_geom0d = new QRadioButton(tr("geometry [DIM=0D]"));
  connect(rb_ctor_geom0d,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_geom0d_checked);

  rb_ctor_geom1d = new QRadioButton(tr("geometry [DIM=1D]"));
  rb_ctor_geom1d->setEnabled(false);
  connect(rb_ctor_geom1d,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_geom1d_checked);

  rb_ctor_geom2d = new QRadioButton(tr("geometry [DIM=2D]"));
  rb_ctor_geom2d->setEnabled(false);
  connect(rb_ctor_geom2d,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_geom2d_checked);

  rb_ctor_geom3d = new QRadioButton(tr("geometry [DIM=3D]"));
  connect(rb_ctor_geom3d,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_geom3d_checked);

  rb_ctor_psg = new QRadioButton(tr("point symmetry group"));
  connect(rb_ctor_psg,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_psg_checked);

  rb_ctor_pgf_prod = new QRadioButton(tr("pgf producer"));
  connect(rb_ctor_pgf_prod,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_pgf_prod_checked);

  rb_ctor_cube = new QRadioButton(tr("cube(parallelepiped) primitive"));
  connect(rb_ctor_cube,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_cube_checked);

  rb_ctor_arrow = new QRadioButton(tr("arrow primitive"));
  connect(rb_ctor_arrow,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_arrow_checked);

  rb_ctor_node_book = new QRadioButton(tr("node book"));
  connect(rb_ctor_node_book,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_node_book_checked);

  rb_ctor_arrow_array = new QRadioButton(tr("arrow array"));
  connect(rb_ctor_arrow_array,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_arrow_array_checked);

  rb_ctor_py_note_book = new QRadioButton(tr("python note book"));
  connect(rb_ctor_py_note_book,
          &QRadioButton::toggled,
          this,
          &add_new_ws_item_widget_t::react_gb_ctor_py_note_book_checked);

  rb_ctor_geom0d->setChecked(true);

  gb_ctor_lt->addWidget(rb_ctor_geom0d);
  gb_ctor_lt->addWidget(rb_ctor_geom1d);
  gb_ctor_lt->addWidget(rb_ctor_geom2d);
  gb_ctor_lt->addWidget(rb_ctor_geom3d);
  gb_ctor_lt->addWidget(rb_ctor_psg);
  gb_ctor_lt->addWidget(rb_ctor_pgf_prod);
  gb_ctor_lt->addWidget(rb_ctor_cube);
  gb_ctor_lt->addWidget(rb_ctor_arrow);
  gb_ctor_lt->addWidget(rb_ctor_node_book);
  gb_ctor_lt->addWidget(rb_ctor_arrow_array);
  gb_ctor_lt->addWidget(rb_ctor_py_note_book);

  auto label_setup = [](QLabel *label, bool visible = false) {
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setMinimumWidth(label_width);
    label->setMinimumHeight(28);
    label->setVisible(visible);
  };

  type_param_ag = new QComboBox(this);
  type_param_ag->setFixedWidth(astate->size_guide.obj_insp_combo_max_w());
  type_param_ag->setVisible(false);
  type_param_ag_lbl = new QLabel(tr("Point group"));
  type_param_ag_lbl->setVisible(false);
  label_setup(type_param_ag_lbl);

  type_cell_ctor_mode = new QComboBox(this);
  type_cell_ctor_mode->setFixedWidth(astate->size_guide.obj_insp_combo_max_w());
  type_cell_ctor_mode->addItem(tr("Cubic cell"));
  type_cell_ctor_mode->addItem(tr("Ortho[3 len]"));
  type_cell_ctor_mode->addItem(tr("3 angles & 3 len"));
  type_cell_ctor_mode->addItem(tr("3 vecs"));
  type_cell_ctor_mode->setVisible(false);
  type_cell_ctor_mode_lbl = new QLabel(tr("Cell mode"));
  label_setup(type_cell_ctor_mode_lbl);

  type_cell_ctor_cubic_size = new QDoubleSpinBox(this);
  type_cell_ctor_cubic_size->setFixedWidth(astate->size_guide.obj_insp_combo_max_w());
  type_cell_ctor_cubic_size->setRange(0.00001, 100);
  type_cell_ctor_cubic_size->setValue(5);
  type_cell_ctor_cubic_size->setSuffix(astate->m_spatial_suffix);
  type_cell_ctor_cubic_size->setVisible(false);
  type_cell_ctor_cubic_size->setAlignment(Qt::AlignCenter);
  type_cell_ctor_cubic_size->setButtonSymbols(QAbstractSpinBox::NoButtons);
  type_cell_ctor_cubic_size->setFixedWidth(astate->size_guide.obj_insp_ctrl_max_w());
  type_cell_ctor_cubic_size->setLocale(QLocale::C);
  type_cell_ctor_cubic_size_lbl = new QLabel(tr("Cube size"));
  label_setup(type_cell_ctor_cubic_size_lbl);

  type_cell_ctor_ortho = new qbinded_float3_input_t(this);
  type_cell_ctor_ortho->set_min_max_step(0.000, 50.0, 0.01);
  type_cell_ctor_ortho->set_default_suffix();
  type_cell_ctor_ortho->bind_value(&m_ortho_g3d_cell);
  type_cell_ctor_ortho->setVisible(false);
  type_cell_ctor_ortho_lbl = new QLabel(tr("Ortho vecs."));
  label_setup(type_cell_ctor_ortho_lbl);

  type_cell_ctor_a3 = new qbinded_float3_input_t(this);
  type_cell_ctor_a3->set_min_max_step(0.000, 180.0, 0.01);
  type_cell_ctor_a3->set_suffix(astate->m_degree_suffix);
  type_cell_ctor_a3->bind_value(&m_a3_g3d_cell);
  type_cell_ctor_a3->setVisible(false);
  type_cell_ctor_a3_lbl = new QLabel(tr("Angles"));
  label_setup(type_cell_ctor_a3_lbl);

  type_cell_ctor_l3 = new qbinded_float3_input_t(this);
  type_cell_ctor_l3->set_min_max_step(0.000, 180.0, 0.01);
  type_cell_ctor_l3->set_default_suffix();
  type_cell_ctor_l3->bind_value(&m_l3_g3d_cell);
  type_cell_ctor_l3->setVisible(false);

  type_cell_ctor_l3_lbl = new QLabel(tr("Lengths"));
  label_setup(type_cell_ctor_l3_lbl);

  type_cell_ctor_a = new qbinded_float3_input_t(this);
  type_cell_ctor_a->set_min_max_step(-50.00, 50.0, 0.01);
  type_cell_ctor_a->set_default_suffix();
  type_cell_ctor_a->bind_value(&m_a_g3d_cell);
  type_cell_ctor_a->setVisible(false);
  type_cell_ctor_a_lbl = new QLabel(tr("a"));
  label_setup(type_cell_ctor_a_lbl);

  type_cell_ctor_b = new qbinded_float3_input_t(this);
  type_cell_ctor_b->set_min_max_step(-50.00, 50.0, 0.01);
  type_cell_ctor_b->set_default_suffix();
  type_cell_ctor_b->bind_value(&m_b_g3d_cell);
  type_cell_ctor_b->setVisible(false);
  type_cell_ctor_b_lbl = new QLabel(tr("b"));
  label_setup(type_cell_ctor_b_lbl);

  type_cell_ctor_c = new qbinded_float3_input_t(this);
  type_cell_ctor_c->set_min_max_step(-50.00, 50.0, 0.01);
  type_cell_ctor_c->set_default_suffix();
  type_cell_ctor_c->bind_value(&m_c_g3d_cell);
  type_cell_ctor_c->setVisible(false);
  type_cell_ctor_c_lbl = new QLabel(tr("c"));
  label_setup(type_cell_ctor_c_lbl);

  connect(type_cell_ctor_mode,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &add_new_ws_item_widget_t::geom3d_ctor_changed);

  type_cell_ctor_mode->setCurrentIndex(0);

  //construct ag_labels
  for (int n = 1; n < 11; n++) {
      std::vector<std::string> nl = shnfl<float>::groups_by_order(n);
      for (auto &elem : nl) type_param_ag->addItem(QString::fromStdString(elem));
    }
  //end of construct ag_labels

  gb_type_param = new qspoiler_widget_t(tr("Type parameters"), nullptr, false);
  //gb_type_param->setMinimumWidth(360);
  gb_type_param_lt = new QFormLayout;
  gb_type_param->add_content_layout(gb_type_param_lt);

  type_param_name = new QLineEdit(this);
  type_param_name->setFixedWidth(astate->size_guide.obj_insp_combo_max_w());
  type_param_name_lbl = new QLabel("Name", this);
  label_setup(type_param_name_lbl, true);

  gb_type_param_lt->addRow(type_param_name_lbl, type_param_name);

  auto cur_ws = astate->ws_mgr->get_cur_ws();
  if (cur_ws) type_param_name->setText(tr("new_item_%1").arg(cur_ws->m_ws_items.size()));

  main_lt->addLayout(data_lt);
  data_lt->addWidget(gb_ctor);
  data_lt->addLayout(sub_data_lt);
  sub_data_lt->addWidget(gb_type_descr);
  sub_data_lt->addWidget(gb_type_param);
  main_lt->addStretch();
  main_lt->addLayout(button_lt);

  setLayout(main_lt);

  connect(ok_button, &QPushButton::clicked,
          this, &add_new_ws_item_widget_t::ok_button_clicked);

  connect(cancel_button, &QPushButton::clicked,
          this, &add_new_ws_item_widget_t::cancel_button_clicked);

}

void add_new_ws_item_widget_t::set_cell_ctors_visibility(bool show) {

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

}

void add_new_ws_item_widget_t::control_top_type_parameters_visibility() {

  //hide all
  set_cell_ctors_visibility(false);
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

}

void add_new_ws_item_widget_t::ok_button_clicked() {

  if (type_param_name->text() == "") {
      int ret = QMessageBox::warning(this,
                                     tr("Missing name!"),
                                     tr("Name is empty! Do something!"));
    } else {

      app_state_t *astate = app_state_t::get_inst();

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

          cell_construct_mode ccm =
              static_cast<cell_construct_mode>(type_cell_ctor_mode->currentIndex());

          auto cur_ws = astate->ws_mgr->get_cur_ws();
          if (!cur_ws) return;
          auto nt =
              astate->ws_mgr->m_bhv_mgr->fbr_ws_item_by_type(geom_view_t::get_type_static());
          auto nt_gv = nt->cast_as<geom_view_t>();
          if (!nt_gv) return;
          nt_gv->m_tws_tr->do_action(act_lock | act_clear_all);
          //nt_gv->m_geom->DIM = 3;
          nt_gv->m_geom->cell.DIM = 3;

          switch (ccm) {

            case cell_construct_mode::construct_cubic : {
                vector3<float> cubic_a{float(type_cell_ctor_cubic_size->value()), 0, 0};
                vector3<float> cubic_b{0, float(type_cell_ctor_cubic_size->value()), 0};
                vector3<float> cubic_c{0, 0, float(type_cell_ctor_cubic_size->value())};
                nt_gv->m_geom->cell = periodic_cell<float>(cubic_a,
                                                           cubic_b,
                                                           cubic_c);
                break;
              }

            case cell_construct_mode::construct_ortho : {
                vector3<float> ortho_a{m_ortho_g3d_cell[0], 0, 0};
                vector3<float> ortho_b{0, m_ortho_g3d_cell[1], 0};
                vector3<float> ortho_c{0, 0, m_ortho_g3d_cell[2]};

                nt_gv->m_geom->cell = periodic_cell<float>(ortho_a,
                                                           ortho_b,
                                                           ortho_c);
                break;
              }

            case cell_construct_mode::construct_3angles_3len : {
                nt_gv->m_geom->cell = periodic_cell<float>(m_l3_g3d_cell[0],
                                                           m_l3_g3d_cell[1],
                                                           m_l3_g3d_cell[2],
                                                           m_a3_g3d_cell[0],
                                                           m_a3_g3d_cell[1],
                                                           m_a3_g3d_cell[2]);
                break;
              }

            case cell_construct_mode::construct_9el : {
                nt_gv->m_geom->cell = periodic_cell<float>(m_a_g3d_cell,
                                                           m_b_g3d_cell,
                                                           m_c_g3d_cell);
                break;
              }

            }

          nt_gv->m_tws_tr->do_action(act_unlock | act_rebuild_tree);
          nt_gv->m_tws_tr->do_action(act_rebuild_ntable);
          nt_gv->m_name = type_param_name->text().toStdString();
          cur_ws->add_item_to_ws(nt);

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

      accept();

    }

}

void add_new_ws_item_widget_t::cancel_button_clicked() {

  reject();

}

void add_new_ws_item_widget_t::react_gb_ctor_geom0d_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      type_descr_lbl->setText(descr_list[0]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_geom1d_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[1]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_geom2d_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[2]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_geom3d_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      type_cell_ctor_mode->setCurrentIndex(0);
      geom3d_ctor_changed(0);
      type_descr_lbl->setText(descr_list[3]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_psg_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[4]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_pgf_prod_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[5]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_cube_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[6]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_arrow_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[7]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_node_book_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[8]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_arrow_array_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[9]);
    }

}

void add_new_ws_item_widget_t::react_gb_ctor_py_note_book_checked(bool checked) {

  control_top_type_parameters_visibility();

  if (checked) {
      set_cell_ctors_visibility(false);
      type_descr_lbl->setText(descr_list[10]);
    }

}

void add_new_ws_item_widget_t::geom3d_ctor_changed(int index) {

  cell_construct_mode ccm = static_cast<cell_construct_mode>(index);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_cubic,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_cubic_size_lbl, type_cell_ctor_cubic_size);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_ortho,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_ortho_lbl, type_cell_ctor_ortho);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_3angles_3len,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_a3_lbl, type_cell_ctor_a3);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_3angles_3len,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_l3_lbl, type_cell_ctor_l3);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_9el,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_c_lbl, type_cell_ctor_c);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_9el,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_b_lbl, type_cell_ctor_b);

  qt_hlp::form_lt_ctrl_visibility(ccm == cell_construct_mode::construct_9el,
                                      gb_type_param_lt,
                                      type_cell_ctor_mode,
                                      type_cell_ctor_a_lbl, type_cell_ctor_a);

}
