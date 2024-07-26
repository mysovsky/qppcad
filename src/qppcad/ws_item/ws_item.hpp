#ifndef QPPCAD_WORKSPACE_ITEM_H
#define QPPCAD_WORKSPACE_ITEM_H

#include <qppcad/core/qppcad.hpp>
#include <geom/lace3d.hpp>
#include <geom/aabb.hpp>
#include <geom/ray.hpp>
#include <qppcad/core/serializable.hpp>
#include <qppcad/core/qpp_object.hpp>
#include <qppcad/core/iupdatable.hpp>

#include <QPainterPath>
#include <QPainter>

namespace qpp {

  namespace cad {

    class workspace_t;
    class app_state_t;
    class ws_item_t;

    struct repair_ws_item_field_t {
        std::shared_ptr<ws_item_t> *m_field;
        std::string m_field_name;
    };

    /*
     *This data structure is necessary to reconstruct links between workspace
     *items after it has been restored from a file
     */
    struct repair_connection_info_t {
        std::map<std::string, std::vector<std::string> > m_connected_items;
        std::map<std::string, std::string> m_leader_con_info;
        std::vector<repair_ws_item_field_t> m_fields;
    };

    // ws_item_t capabilities
    const uint32_t ws_item_flags_default                  = 0;
    const uint32_t ws_item_flags_support_tr               = 1 << 1;
    const uint32_t ws_item_flags_support_scl              = 1 << 2;
    const uint32_t ws_item_flags_support_rot              = 1 << 3;
    const uint32_t ws_item_flags_support_cnt_edit         = 1 << 4;
    const uint32_t ws_item_flags_support_sel              = 1 << 5;
    const uint32_t ws_item_flags_support_render_bb        = 1 << 6;
    const uint32_t ws_item_flags_toolbar_extension        = 1 << 7;
    const uint32_t ws_item_flags_support_actions          = 1 << 8;
    const uint32_t ws_item_flags_support_delete           = 1 << 9;
    const uint32_t ws_item_flags_support_clone            = 1 << 10;
    const uint32_t ws_item_flags_support_moveto           = 1 << 11;
    const uint32_t ws_item_flags_support_rendering        = 1 << 12;
    const uint32_t ws_item_flags_support_extended_editor  = 1 << 13;
    const uint32_t ws_item_flags_translate_emit_upd_event = 1 << 14;
    const uint32_t ws_item_flags_support_view_voting      = 1 << 15;
    const uint32_t ws_item_flags_cam_target_view          = 1 << 16;
    const uint32_t ws_item_flags_fetch_leader_pos         = 1 << 17;

    // ws_item_t update_externally flags
    const uint32_t ws_item_updf_flags                   = 0;
    const uint32_t ws_item_updf_generic                 = 1 << 1;
    const uint32_t ws_item_updf_pos_changed             = 1 << 2;
    const uint32_t ws_item_updf_scale_changed           = 1 << 3;
    const uint32_t ws_item_updf_leader_changed          = 1 << 4;
    const uint32_t ws_item_updf_followers_changed       = 1 << 5;
    const uint32_t ws_item_updf_connected_items_changed = 1 << 6;
    const uint32_t ws_item_updf_added_to_ws             = 1 << 7;
    const uint32_t ws_item_updf_removed_from_ws         = 1 << 8;
    const uint32_t ws_item_updf_pre_delete              = 1 << 9;
    const uint32_t ws_item_updf_channel_all             = 1 << 10;
    const uint32_t ws_item_updf_channel_a               = 1 << 11;
    const uint32_t ws_item_updf_channel_b               = 1 << 12;
    const uint32_t ws_item_updf_channel_c               = 1 << 13;
    const uint32_t ws_item_updf_channel_d               = 1 << 14;
    const uint32_t ws_item_updf_channel_e               = 1 << 15;
    const uint32_t ws_item_updf_channel_f               = 1 << 16;
    const uint32_t ws_item_updf_channel_g               = 1 << 17;
    const uint32_t ws_item_updf_channel_h               = 1 << 18;
    const uint32_t ws_item_updf_channel_j               = 1 << 19;
    const uint32_t ws_item_updf_channel_k               = 1 << 20;
    const uint32_t ws_item_updf_channel_l               = 1 << 21;
    const uint32_t ws_item_updf_regenerate_content      = 1 << 22;

    const uint8_t ws_item_render_order_0   = 0;
    const uint8_t ws_item_render_order_1   = 1;
    const uint8_t ws_item_render_order_2   = 2;
    const uint8_t ws_item_render_order_3   = 3;
    const uint8_t ws_item_render_order_4   = 4;
    const uint8_t ws_item_render_order_5   = 5;
    const uint8_t ws_item_render_order_max = 6;

    class ws_item_t :
        public std::enable_shared_from_this<ws_item_t>,
        public qpp_object_t,
        public iupdatable_t {

        QPP_OBJECT(ws_item_t, qpp_object_t)

      private:

          uint32_t p_flags{0};

      public:

        int m_render_order{0};

        workspace_t *m_parent_ws{};
        std::vector<std::shared_ptr<ws_item_t> > m_connected_items;
        std::vector<std::shared_ptr<ws_item_t> > m_followers;
        std::shared_ptr<ws_item_t> m_leader{nullptr};

        std::string      m_name;
        std::string      m_genesis_file_name;
        aabb_3d_t<float> m_aabb;
        vector3<float>   m_leader_offset{0, 0, 0};
        vector3<float>   m_pos_old;
        vector3<float>   m_pos{0.0f, 0.0f, 0.0f};
        vector3<float>   m_scale{1.0f, 1.0f, 1.0f};
        vector3<float>   m_rotation{1.0f, 1.0f, 1.0f};
        vector3<float>   explicit_translation;

        int m_last_tab{-1};
        bool m_is_visible{true};
        bool m_show_bb{true}; // whether to show object`s bounding box
        bool m_selected{false};
        bool m_marked_for_deletion{false};
        bool m_extended_editor_opened{false};
        size_t m_extended_editor_id{0};
        size_t m_extended_editor_width{300};

        vector3<float> get_pos();
        void set_pos(vector3<float> new_pos);
        virtual void vote_for_view_vectors(vector3<float> &out_look_pos,
                                           vector3<float> &out_look_at) = 0;

        virtual void target_view(cam_tv_e target_view_src,
                                 vector3<float> &look_from,
                                 vector3<float> &look_to,
                                 vector3<float> &look_up,
                                 bool &need_to_update_camera);

        void apply_target_view(cam_tv_e target_view_src);

        /**
         * @brief set_parent_workspace
         * @param _parent_ws
         */
        void set_parent_workspace(workspace_t *_parent_ws);

        /**
         * @brief get_name
         * @return
         */
        const std::string get_name();

        /**
         * @brief set_name
         * @param _name
         */
        void set_name(const std::string &_name);

        /**
         * @brief set_name
         * @param _name
         */
        void set_name(const char * _name);

        // "many-to-many" data relationship
        void add_connected_item(std::shared_ptr<ws_item_t> new_item);
        void rm_connected_item(std::shared_ptr<ws_item_t> item_to_remove);
        std::optional<size_t> get_connected_idx(std::shared_ptr<ws_item_t> item_to_find);
        bool is_connected(std::shared_ptr<ws_item_t> item_to_find);

        // "one-to-many" data relationship
        void add_follower(std::shared_ptr<ws_item_t> new_item);
        std::optional<size_t> get_follower_idx(std::shared_ptr<ws_item_t> item_to_find);
        bool is_follower(std::shared_ptr<ws_item_t> item_to_find);
        void rm_follower(std::shared_ptr<ws_item_t> item_to_remove);

        void set_bounded_to_leader(bool bounding);
        bool is_bounded();

        bool is_selected();

        virtual void render();
        virtual void render_overlay(QPainter &painter);

        virtual size_t get_content_count() = 0;
        virtual bool mouse_click(ray_t<float> *ray, bool pressed) = 0;
        virtual void mouse_double_click(ray_t<float> *ray);

        void set_default_flags(uint32_t);
        uint32_t get_flags() const ;
        void set_flag(uint32_t flag);
        void unset_flag(uint32_t flag);

        virtual uint32_t get_num_cnt_selected() = 0;

        virtual std::string compose_type_descr() = 0;

        virtual void on_leader_changed();
        virtual void on_leader_call();
        void call_followers();

        /**
         * @brief update
         * @param delta_time
         */
        virtual void update(float delta_time);

        /**
         * @brief get_bb_prescaller
         * @return
         */
        virtual float get_bb_prescaller();
        virtual bool is_bb_visible();

        /**
         * @brief get_gizmo_content_barycenter
         * @return
         */
        virtual const vector3<float> get_gizmo_content_barycenter();

        void updated_externally(uint32_t update_reason = ws_item_updf_generic) override;

        virtual void on_begin_node_gizmo_translate();
        virtual void on_end_node_gizmo_translate();
        virtual void on_begin_content_gizmo_translate();
        virtual void apply_intermediate_translate_content(const vector3<float> &new_pos);
        virtual void on_end_content_gizmo_translate();
        void translate(const vector3<float> &tr_vec);

        virtual void save_to_json(json &data) ;
        virtual void load_from_json(json &data, repair_connection_info_t &rep_info);
        virtual bool can_be_written_to_json();
        void save_ws_item_field(const std::string &field_name,
                                std::shared_ptr<ws_item_t> field_ws_item,
                                json &data);
        void load_ws_item_field(const std::string &field_name,
                                std::shared_ptr<ws_item_t> *field_ws_item,
                                json &data,
                                repair_connection_info_t &rep_info);

        void load_from_stream(std::basic_istream<CHAR_EX,TRAITS> &stream);
        void save_to_stream(std::basic_istream<CHAR_EX,TRAITS> &stream);

        void load_from_file(std::string &file_name);
        void save_from_file(std::string &file_name);

        void update_oi();
        std::string py_get_repr();

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
