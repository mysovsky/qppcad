#ifndef QPPCAD_WS_ARROW_PRIMITIVE
#define QPPCAD_WS_ARROW_PRIMITIVE

#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/ws_item.hpp>

namespace qpp {

  namespace cad {

    class arrow_primitive_t final: public ws_item_t {

        QPP_OBJECT(arrow_primitive_t, ws_item_t)

      public:

        matrix4<float> m_model_body;
        matrix4<float> m_model_arrow;
        vector3<float> m_arrow_to{1, 1, 1};
        vector3<float> m_color{1, 0, 0};

        float m_arrow_cap_len{0.15f};
        float m_arrow_scale{0.05f};
        float m_arrow_cap_scale{0.12f};

        bool m_need_to_recalc_render_data{true};

        arrow_primitive_t();

        void vote_for_view_vectors(vector3<float> &out_look_pos,
                                   vector3<float> &out_look_at) override ;
        void render() override;
        bool mouse_click(ray_t<float> *click_ray, bool pressed) override;

        std::string compose_type_descr() override;
        void update(float delta_time) override;
        float get_bb_prescaller() override;
        void updated_externally(uint32_t update_reason) override;
        uint32_t get_num_cnt_selected() override;
        size_t get_content_count() override;
        bool can_be_written_to_json() override;

        void save_to_json(json &data) override;
        void load_from_json(json &data, repair_connection_info_t &rep_info) override;

        void recalc_render_data();

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
