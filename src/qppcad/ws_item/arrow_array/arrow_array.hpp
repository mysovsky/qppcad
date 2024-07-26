#ifndef QPPCAD_ARROW_ARRAY
#define QPPCAD_ARROW_ARRAY
#include <qppcad/core/qppcad.hpp>
#include <data/color.hpp>
#include <geom/geom.hpp>
#include <qppcad/ws_item/ws_item.hpp>

namespace qpp {

  // ---   asm ------
  // Move this to qpp !!!

  template <class REAL>
  class geom_atom_vectors {
  public:

    geometry<REAL,periodic_cell<REAL> > * binded_geom;
    std::vector<vector3<REAL> > vectors;

    geom_atom_vectors(geometry<REAL,periodic_cell<REAL> > * __binded_geom = nullptr){
      binded_geom = __binded_geom;
      if (binded_geom){
	vectors.resize(binded_geom -> nat());
	std::fill(vectors.begin(), vectors.end(), vector3<REAL>(0e0));
      }
    }

    geom_atom_vectors(const std::vector<vector3<REAL> > & __vectors,
		      geometry<REAL,periodic_cell<REAL> > * __binded_geom) :
      vectors(__vectors), binded_geom(__binded_geom) {}

    bool is_valid() const{
      if (binded_geom)
	if (vectors.size() == binded_geom->nat() )
	  return true;
      return false;
    }

    vector3<REAL> start_pos(int i) const{
      return binded_geom->pos(i);
    }

    vector3<REAL> end_pos(int i, REAL scale = 1e0) const{
      return binded_geom->pos(i) + scale*vectors[i];
    }
    
    vector3<REAL> start_pos(int i, const index &I) const{
      return binded_geom->pos(i,I);
    }

    vector3<REAL> end_pos(int i, const index &I, REAL scale = 1e0) const{
      return binded_geom->pos(i,I) + scale*vectors[i];
    }
    
  };

  
  namespace cad {

    class geom_view_t;

    class arrow_array_view_t final: public ws_item_t {

        QPP_OBJECT(arrow_array_view_t, ws_item_t)

      public:

        geom_view_t *m_binded_gv{nullptr};
        std::shared_ptr<geom_atom_vectors<float> > m_binded_vectors{nullptr};
      
        std::shared_ptr<ws_item_t> m_src{nullptr};

        float m_unf_arrow_len{1.0f};
        float m_unf_arrow_cap_len{0.15f};
        float m_unf_arrow_scale{0.05f};
        float m_unf_arrow_cap_scale{0.12f};

        float m_displ_eps{0.0005f};

        bool m_affected_by_sv{false};
        vector3<float> m_color{clr_red};

        arrow_array_view_t();

        void vote_for_view_vectors(vector3<float> &out_look_pos,
                                   vector3<float> &out_look_at) override ;

        void create_zero_vectors();
        void create_vectors_from_frames(int f1, int f2);
        void create_vectors_from_fields(int f1, int f2, int f3);
      
        void render() override;
        bool mouse_click(ray_t<float> *click_ray, bool pressed) override;

        std::string compose_type_descr() override;
        void update(float delta_time) override;
        float get_bb_prescaller() override;
        void updated_externally(uint32_t update_reason) override;
        uint32_t get_num_cnt_selected() override;
        size_t get_content_count() override;

        void save_to_json(json &data) override;
        void load_from_json(json &data, repair_connection_info_t &rep_info) override;
        bool can_be_written_to_json() override;

    };

  } // namespace qpp::cad

} // namespace qpp

#endif
