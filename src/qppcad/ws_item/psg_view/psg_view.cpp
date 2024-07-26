#include <qppcad/ws_item/psg_view/psg_view.hpp>
#include <qppcad/core/app_state.hpp>

using namespace qpp;
using namespace qpp::cad;

psg_view_t::psg_view_t() {

  set_default_flags(ws_item_flags_default |
                    ws_item_flags_support_tr |
                    ws_item_flags_translate_emit_upd_event);

  //set default axes colors by order

  m_axes_color_by_order[0]  = {  32.0 / 255.0,  32.0 / 255.0,  32.0 / 255.0 };
  m_axes_color_by_order[1]  = {  62.0 / 255.0,   6.0 / 255.0,   7.0 / 255.0 };
  m_axes_color_by_order[2]  = {  55.0 / 255.0, 126.0 / 255.0, 184.0 / 255.0 };
  m_axes_color_by_order[3]  = {  77.0 / 255.0, 175.0 / 255.0,  74.0 / 255.0 };
  m_axes_color_by_order[4]  = { 152.0 / 255.0,  78.0 / 255.0, 163.0 / 255.0 };
  m_axes_color_by_order[5]  = { 255.0 / 255.0, 127.0 / 255.0,   0.0 / 255.0 };
  m_axes_color_by_order[6]  = { 255.0 / 255.0, 255.0 / 255.0,  51.0 / 255.0 };
  m_axes_color_by_order[7]  = { 166.0 / 255.0,  86.0 / 255.0,  40.0 / 255.0 };
  m_axes_color_by_order[8]  = { 247.0 / 255.0, 129.0 / 255.0, 191.0 / 255.0 };
  m_axes_color_by_order[9]  = { 166.0 / 255.0, 206.0 / 255.0, 227.0 / 255.0 };
  //m_axes_color_by_order[10] = {  31.0 / 255.0, 120.0 / 255.0, 180.0 / 255.0 };

}

void psg_view_t::gen_from_geom(xgeometry<float, periodic_cell<float> > &geom,
                               float tolerance,
                               bool nested) {

  if (!m_ag) m_ag = std::make_shared<array_group<matrix3<float> > > ();
  find_point_symm(*m_ag, geom, m_leader_offset, tolerance);

  updated_externally(ws_item_updf_leader_changed); //fetch bounded position from leader(geom_view)
  regenerate_atf();
  recalc_render_data();

}

void psg_view_t::recalc_render_data() {

  for (auto &elem : m_atf) {

      vector3<float> aliasing {
        (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))*0.05f
      };

      if (elem.m_is_plane) {

          matrix4<float> mat_model = matrix4<float>::Identity();
          vector3<float> plane_normal_ut = vector3<float>(0, 0, 1);
          vector3<float> plane_rot_axis = plane_normal_ut.cross(elem.m_axis).normalized();
          float len_mod = elem.m_axis.norm() * plane_normal_ut.norm();
          float rot_angle = std::acos(plane_normal_ut.dot(elem.m_axis) / len_mod);
          Eigen::Affine3f t;
          t = Eigen::AngleAxisf(rot_angle, plane_rot_axis);
          t.prescale(vector3<float>(m_plane_scale));
          t.pretranslate(m_pos);
          mat_model = t.matrix();
          elem.m_arrow_body_mat = mat_model;

        } else {

          vector3<float> dir = elem.m_axis.normalized();

          vector3<float> start = dir * (-0.5f * m_arrow_len) + m_pos;
          vector3<float> end   = dir * ( 0.5f * m_arrow_len) + m_pos;

          if (elem.m_roto_inv) elem.m_ri_cube_center = start;

          matrix4<float> mat_model = matrix4<float>::Identity();

          mat_model.block<3,1>(0,3) = start;
          mat_model.block<3,1>(0,2) = end - start;

          vector3<float> vec_axis_norm = mat_model.block<3,1>(0,2).normalized();

          mat_model.block<3,1>(0,0) = vec_axis_norm.unitOrthogonal() * m_arrow_scale;
          mat_model.block<3,1>(0,1) = vec_axis_norm.cross(mat_model.block<3,1>(0,0));
          mat_model.block<3,1>(0,3) = start ;

          elem.m_arrow_body_mat = mat_model;

          //aux
          vector3<float> start_aux = end;
          vector3<float> end_aux = end + dir * m_arrow_cap_len;

          matrix4<float> mat_model_aux = matrix4<float>::Identity();
          mat_model_aux.block<3,1>(0,3) = start_aux;
          mat_model_aux.block<3,1>(0,2) = end_aux - start_aux;

          vector3<float> vec_axis_norm_aux = mat_model_aux.block<3,1>(0,2).normalized();
          mat_model_aux.block<3,1>(0,0) = vec_axis_norm_aux.unitOrthogonal() *
                                          m_arrow_cap_scale;
          mat_model_aux.block<3,1>(0,1) = vec_axis_norm_aux.cross(mat_model_aux.block<3,1>(0,0));
          mat_model_aux.block<3,1>(0,3) = start_aux;

          elem.m_arrow_cap_mat = mat_model_aux;

        }

    }

  update_axes_color();

}

void psg_view_t::vote_for_view_vectors(vector3<float> &out_look_pos,
                                       vector3<float> &out_look_at) {
  //do nothing
}

void psg_view_t::render() {

  if (!m_is_visible) return;

  app_state_t* astate = app_state_t::get_inst();

  //draw axes
  astate->dp->begin_render_general_mesh();

  if (m_show_axes)
    for (auto &elem : m_atf)
      if (!elem.m_is_plane && elem.m_is_visible) {
          astate->dp->render_general_mesh(elem.m_arrow_body_mat,
                                          elem.m_color,
                                          astate->mesh_cylinder);
          astate->dp->render_general_mesh(elem.m_arrow_cap_mat,
                                          elem.m_color,
                                          astate->mesh_unit_cone);
          if (elem.m_roto_inv)
            astate->dp->render_general_mesh(elem.m_ri_cube_center,
                                            vector3<float>{m_arrow_scale * 1.2f},
                                            vector3<float>{0},
                                            elem.m_color,
                                            astate->mesh_unit_cube);
        }

  astate->dp->end_render_general_mesh();

  if (m_show_planes) {

      if (m_plane_alpha_enabled) {

          astate->dp->begin_render_general_mesh(astate->sp_mvap_ssl);
          astate->dp->cull_func(draw_pipeline_cull_func::cull_disable);
          for (auto &elem : m_atf)
            if (elem.m_is_plane && elem.m_is_visible) {
                astate->dp->render_general_mesh(elem.m_arrow_body_mat,
                                                elem.m_color,
                                                astate->mesh_zl_plane,
                                                m_plane_alpha,
                                                astate->sp_mvap_ssl);
              }
          astate->dp->cull_func(draw_pipeline_cull_func::cull_enable);
          astate->dp->end_render_general_mesh(astate->sp_mvap_ssl);

        } else {
          astate->dp->begin_render_general_mesh();
          astate->dp->cull_func(draw_pipeline_cull_func::cull_disable);
          for (auto &elem : m_atf)
            if (elem.m_is_plane && elem.m_is_visible) {
                astate->dp->render_general_mesh(elem.m_arrow_body_mat,
                                                elem.m_color,
                                                astate->mesh_zl_plane);
              }
          astate->dp->cull_func(draw_pipeline_cull_func::cull_enable);
          astate->dp->end_render_general_mesh();
        }

    }

}

bool psg_view_t::mouse_click(ray_t<float> *click_ray, bool pressed) {
  return false;
}

std::string psg_view_t::compose_type_descr() {
  return "point sym. group view";
}

void psg_view_t::update(float delta_time) {
  //do nothing
}

float psg_view_t::get_bb_prescaller() {
  return 1.0f;
}

void psg_view_t::updated_externally(uint32_t update_reason) {

  ws_item_t::updated_externally(update_reason);
  recalc_render_data();
 // update_axes_color();

}

void psg_view_t::on_leader_call() {

}

uint32_t psg_view_t::get_num_cnt_selected() {
  return 0;
}

size_t psg_view_t::get_content_count() {
  return 0;
}

void psg_view_t::save_to_json(json &data) {

  ws_item_t::save_to_json(data);

}

void psg_view_t::load_from_json(json &data, repair_connection_info_t &rep_info) {

  ws_item_t::load_from_json(data, rep_info);

}

void psg_view_t::regenerate_atf() {

  app_state_t* astate = app_state_t::get_inst();
  m_atf.clear();
  if (!m_ag) return;
  m_pg_axes = point_group_axes<float>(*m_ag);

  astate->tlog("pg_axes_info: inversion ? = {}", m_pg_axes.inversion);
  astate->tlog("pg_axes_info: axes.size() = {}", m_pg_axes.axes.size());
  astate->tlog("pg_axes_info: planes.size() = {}", m_pg_axes.planes.size());

  astate->tlog("pg_axes_info: rotoinversion.size() = {}", m_pg_axes.rotoinversion.size());
  for (size_t i = 0; i < m_pg_axes.rotoinversion.size(); i++)
    astate->tlog("pg_axes_info: rotoinversion[{}] = {}", i, m_pg_axes.rotoinversion[i]);

  astate->tlog("pg_axes_info: orders.size() = {}", m_pg_axes.orders.size());
  for (size_t i = 0; i < m_pg_axes.orders.size(); i++)
    astate->tlog("pg_axes_info: orders[{}] = {}", i, m_pg_axes.orders[i]);

  for (int i = 0; i < m_pg_axes.axes.size(); i++) {

      transform_record_t new_tr;
      new_tr.m_is_plane = false;
      new_tr.m_axis = m_pg_axes.axes[i];
      new_tr.m_order = m_pg_axes.orders[i];
      new_tr.m_roto_inv = m_pg_axes.rotoinversion[i];
      m_atf.push_back(std::move(new_tr));
    }

  for (int i = 0; i < m_pg_axes.planes.size(); i++) {
      transform_record_t new_tr;
      new_tr.m_is_plane = true;
      new_tr.m_axis =  m_pg_axes.planes[i];
      new_tr.m_color = {0, 1, 0};
      m_atf.push_back(std::move(new_tr));
    }

}

void psg_view_t::update_view() {

  app_state_t* astate = app_state_t::get_inst();

  regenerate_atf();
  recalc_render_data();

  astate->astate_evd->cur_ws_selected_item_changed();
  astate->make_viewport_dirty();

}

void psg_view_t::update_axes_color() {

  for (size_t i = 0; i < m_atf.size(); i++)
    if (!m_atf[i].m_is_plane) {
        size_t clr_lk = std::clamp<size_t>(m_atf[i].m_order - 1, 0, AXIS_COLORIZE_SIZE);
        m_atf[i].m_color =  m_axes_color_by_order[clr_lk];
      } else {
        m_atf[i].m_color = m_plane_color;
      }

}



