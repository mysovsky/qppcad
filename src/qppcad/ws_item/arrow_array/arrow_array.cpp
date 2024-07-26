#include <qppcad/ws_item/arrow_array/arrow_array.hpp>
#include <qppcad/ws_item/geom_view/geom_view.hpp>
#include <qppcad/ws_item/geom_view/geom_view_anim_subsys.hpp>
#include <qppcad/core/app_state.hpp>
#include <qppcad/core/json_helpers.hpp>

using namespace qpp;
using namespace qpp::cad;

arrow_array_view_t::arrow_array_view_t() {
  set_default_flags(ws_item_flags_default | ws_item_flags_support_rendering);
}

void arrow_array_view_t::vote_for_view_vectors(vector3<float> &out_look_pos,
                                          vector3<float> &out_look_at) {
  //do nothing
}

void arrow_array_view_t::create_zero_vectors()
{
  if (!m_binded_gv) return;  
  if (!m_binded_gv -> m_geom) return;  
  m_binded_vectors = std::make_shared<geom_atom_vectors<float> >(&(*m_binded_gv -> m_geom));
}

//------------------------------------
void arrow_array_view_t::create_vectors_from_frames(int f1, int f2)
{
  //  if (!m_is_visible) return;
  if (!m_binded_gv) return;  
  if (!m_binded_gv -> m_geom) return;  
  
  auto geom = m_binded_gv -> m_geom;
  // Create zero vectors
  m_binded_vectors = std::make_shared<geom_atom_vectors<float> >(&(*geom));
   
  if (!m_binded_gv->m_anim->animable()) return;
     
  auto cur_anim = m_binded_gv->m_anim->get_current_anim();

  if (cur_anim && cur_anim->m_anim_type == geom_anim_t::anim_static)
    return;
  if (cur_anim->frames.size() <= f1 || cur_anim->frames.size() <= f2) return;

  for (size_t i = 0; i < geom -> nat(); i++) {
    
    m_binded_vectors -> vectors[i] = cur_anim -> frames[f2].atom_pos[i] - cur_anim -> frames[f1].atom_pos[i];
  }

  /* WTF is all this? I can't understand - asm

  int total_frames    = cur_anim->frames.size();
  float start_frame_0 = int(m_binded_gv->m_anim->m_cur_anim_time);
  float end_frame_0   = std::ceil(m_binded_gv->m_anim->m_cur_anim_time);
  float frame_delta_0 = 1 - (m_binded_gv->m_anim->m_cur_anim_time - start_frame_0);
  int start_frame_n_0 = std::clamp(int(start_frame_0), 0, total_frames-1);
  int end_frame_n_0   = std::clamp(int(end_frame_0), 0, total_frames-1);

  float integrated_dt = 1.01 / (m_binded_gv->m_anim->m_anim_frame_time * 60);
  float start_frame_1 = int(m_binded_gv->m_anim->m_cur_anim_time + integrated_dt);
  float end_frame_1   = std::ceil(m_binded_gv->m_anim->m_cur_anim_time + integrated_dt);
  float frame_delta_1 = 1 - (m_binded_gv->m_anim->m_cur_anim_time + integrated_dt - start_frame_1);
  int start_frame_n_1 = std::clamp(int(start_frame_1), 0, total_frames-1);
  int end_frame_n_1   = std::clamp(int(end_frame_1), 0, total_frames-1);

  if (start_frame_0 >= cur_anim->frames.size() || end_frame_n_0 >= cur_anim->frames.size()) return;
  if (cur_anim->frames[start_frame_n_0].atom_pos.size() != m_binded_gv->m_geom->nat()) return;
  if (cur_anim->frames[end_frame_n_0].atom_pos.size() != m_binded_gv->m_geom->nat()) return;
  if (cur_anim->frames[start_frame_n_1].atom_pos.size() != m_binded_gv->m_geom->nat()) return;
  if (cur_anim->frames[end_frame_n_1].atom_pos.size() != m_binded_gv->m_geom->nat()) return;

//  astate->log(fmt::format("ENTER AA RENDERING s0={} e0={} d0={} s1={} e1={} d1={}",
//                          start_frame_0, end_frame_0, frame_delta_0,
//                          start_frame_1, end_frame_1, frame_delta_1));
*/

      /*
      vector3<float> start_pos = cur_anim->frames[start_frame_n_0].atom_pos[i] * (frame_delta_0) +
                                 cur_anim->frames[end_frame_n_0].atom_pos[i] * (1-frame_delta_0) +
                                 m_binded_gv->m_pos;

      vector3<float> end_pos = cur_anim->frames[start_frame_n_1].atom_pos[i] * (frame_delta_1) +
                               cur_anim->frames[end_frame_n_1].atom_pos[i] * (1-frame_delta_1) +
                               m_binded_gv->m_pos;

      float dnodem = (end_pos - start_pos).norm();
      if (dnodem < m_displ_eps) continue;

      vector3<float> dir = (end_pos - start_pos).normalized();
      // ---------------- asm -------------------------------
      //vector3<float> end = start_pos + dir * m_unf_arrow_len;
      vector3<float> end = start_pos + dir * m_unf_arrow_len * dnodem * 10.0;
      
  }*/
  
}

void arrow_array_view_t::create_vectors_from_fields(int f1, int f2, int f3){
  if (!m_binded_gv) return;  
  if (!m_binded_gv -> m_geom) return;  
  
  auto geom = m_binded_gv -> m_geom;
  m_binded_vectors = std::make_shared<geom_atom_vectors<float> >(&(*geom));
  
  for (size_t i = 0; i < geom -> nat(); i++) {
    
    m_binded_vectors -> vectors[i] =
      vector3<float>(geom -> xfield<float>(f1,i), geom -> xfield<float>(f2,i),  geom -> xfield<float>(f3,i));
  }
}


void arrow_array_view_t::render() {

  app_state_t* astate = app_state_t::get_inst();

  if (!m_is_visible) return;

  ws_item_t::render();

      
  //-------------------------------------------
  astate->dp->begin_render_general_mesh();
  for (size_t i = 0; i < m_binded_gv->m_geom->nat(); i++) {

    if (m_affected_by_sv && m_binded_gv->m_geom->xfield<bool>(xgeom_sel_vis_hide, i)) continue;
    
      vector3<float> start_pos = m_binded_vectors->start_pos(i);
      vector3<float> end = m_binded_vectors->end_pos(i,m_unf_arrow_len - m_unf_arrow_cap_len);
      vector3<float> dir = (end - start_pos).normalized();
      float val = m_binded_vectors->vectors[i].norm();
      if (val < m_displ_eps) continue;
 
      matrix4<float> mat_body = matrix4<float>::Identity();
      mat_body.block<3,1>(0,3) = start_pos;
      mat_body.block<3,1>(0,2) = end - start_pos;

      vector3<float> vec_axis_norm = mat_body.block<3,1>(0,2).normalized();
      mat_body.block<3,1>(0,0) = vec_axis_norm.unitOrthogonal() * m_unf_arrow_scale;
      mat_body.block<3,1>(0,1) = vec_axis_norm.cross(mat_body.block<3,1>(0,0));
      mat_body.block<3,1>(0,3) = start_pos ;

      vector3<float> start_ar = end;
      vector3<float> end_ar = end + dir * m_unf_arrow_cap_len;

      matrix4<float> mat_arrow = matrix4<float>::Identity();
      mat_arrow.block<3,1>(0,3) = start_ar;
      mat_arrow.block<3,1>(0,2) = end_ar - start_ar;

      vector3<float> veca_axis_norm = mat_arrow.block<3,1>(0,2).normalized();
      mat_arrow.block<3,1>(0,0) = veca_axis_norm.unitOrthogonal() * m_unf_arrow_cap_scale;
      mat_arrow.block<3,1>(0,1) = veca_axis_norm.cross(mat_arrow.block<3,1>(0,0));
      mat_arrow.block<3,1>(0,3) = start_ar ;

      astate->dp->render_general_mesh(mat_body, m_color, astate->mesh_cylinder);
      astate->dp->render_general_mesh(mat_arrow, m_color, astate->mesh_unit_cone);

    }

  astate->dp->end_render_general_mesh();

}

bool arrow_array_view_t::mouse_click(ray_t<float> *click_ray, bool pressed) {
  return false;
}

std::string arrow_array_view_t::compose_type_descr() {
  return "arrow_array";
}

void arrow_array_view_t::update(float delta_time) {
  //do nothing
}

float arrow_array_view_t::get_bb_prescaller() {
  return 1.0f;
}

uint32_t arrow_array_view_t::get_num_cnt_selected() {
  return 0;
}

size_t arrow_array_view_t::get_content_count() {
  return 0;
}

void arrow_array_view_t::save_to_json(json &data) {

  ws_item_t::save_to_json(data);

  save_ws_item_field(JSON_AARRAY_SRD, m_src, data);
  json_helper::save_var(JSON_AARRAY_ARROW_LEN, m_unf_arrow_len, data);
  json_helper::save_var(JSON_AARRAY_ARROW_CAP_LEN, m_unf_arrow_cap_len, data);
  json_helper::save_var(JSON_AARRAY_ARROW_SCALE, m_unf_arrow_scale, data);
  json_helper::save_var(JSON_AARRAY_ARROW_CAP_SCALE, m_unf_arrow_cap_scale, data);
  json_helper::save_var(JSON_AARRAY_AFFECTED_BY_SV, m_affected_by_sv, data);
  json_helper::save_vec3(JSON_AARRAY_COLOR, m_color, data);

}

void arrow_array_view_t::load_from_json(json &data, repair_connection_info_t &rep_info) {

  ws_item_t::load_from_json(data, rep_info);
  load_ws_item_field(JSON_AARRAY_SRD, &m_src, data, rep_info);
  json_helper::load_var(JSON_AARRAY_ARROW_LEN, m_unf_arrow_len, data);
  json_helper::load_var(JSON_AARRAY_ARROW_CAP_LEN, m_unf_arrow_cap_len, data);
  json_helper::load_var(JSON_AARRAY_ARROW_SCALE, m_unf_arrow_scale, data);
  json_helper::load_var(JSON_AARRAY_ARROW_CAP_SCALE, m_unf_arrow_cap_scale, data);
  json_helper::load_var(JSON_AARRAY_AFFECTED_BY_SV, m_affected_by_sv, data);
  json_helper::load_vec3(JSON_AARRAY_COLOR, m_color, data);

}

bool arrow_array_view_t::can_be_written_to_json() {
  return true;
}

void arrow_array_view_t::updated_externally(uint32_t update_reason) {

  ws_item_t::updated_externally(update_reason);

  app_state_t *astate = app_state_t::get_inst();

  if (m_src) {
      auto _as_gv = m_src->cast_as<geom_view_t>();
      if (_as_gv) m_binded_gv = _as_gv;
      else m_binded_gv = nullptr;
    } else {
      m_binded_gv = nullptr;
    }

}

