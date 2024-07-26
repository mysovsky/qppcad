#ifndef QPPCAD_WS_VOLUME_DATA
#define QPPCAD_WS_VOLUME_DATA

#include <qppcad/core/qppcad.hpp>
#include <qppcad/ws_item/ws_item.hpp>
#include <io/cube.hpp>
#include <qppcad/render/mesh.hpp>
#include <data/color.hpp>

namespace qpp {

  namespace cad {

    enum ws_volume_t  : int {
      volume_mo,
      volume_density
    };

    class volume_view_t;

    class ws_volume_record_t {

      public:

        volume_view_t *m_owner{nullptr};

        scalar_volume_t<float> m_volume;
        mesh_t *m_first_mesh{nullptr};
        mesh_t *m_second_mesh{nullptr};

        bool m_ready_to_render{false};
        bool m_need_to_regenerate{false};
        bool m_transparent_volume{false};
        bool m_render_permanent{false};
        float m_alpha{0.75f};
        float m_isolevel{qpp::def_isovalue_dens};

        vector3<float> m_color_pos{clr_red};
        vector3<float> m_color_neg{clr_navy};
        vector3<float> m_color_vol{clr_yellow};

        ws_volume_t m_volume_type;

        size_t m_state_id{0};
        size_t m_spin_subspace{0};

        void copy_from(ws_volume_record_t &other, bool clear_volume = false);

    };

    class volume_view_t final: public ws_item_t {

        QPP_OBJECT(volume_view_t, ws_item_t)

      public:

        std::vector<std::shared_ptr<ws_volume_record_t> > m_volumes;
        int m_current_volume{0};

        void mc_polygonise(float _isolevel);

        volume_view_t();

        void vote_for_view_vectors(vector3<float> &v_out_look_pos,
                                   vector3<float> &v_out_look_at) override ;
        void render() override;
        bool mouse_click(ray_t<float> *click_ray, bool pressed) override;

        std::string compose_type_descr() override;
        void update(float delta_time) override;
        float get_bb_prescaller() override;

        uint32_t get_num_cnt_selected() override;
        size_t get_content_count() override;
        void updated_externally(uint32_t update_reason) override;

        void save_to_json(json &data) override;
        void load_from_json(json &data, repair_connection_info_t &rep_info) override;

        size_t clone_volume(size_t volume_id);

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
