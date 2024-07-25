#include <qppcad/ws_item/geom_view/qtype_summary_model.hpp>
#include <qppcad/core/app_state.hpp>
#include <iterator>

using namespace qpp;
using namespace qpp::cad;

int qtype_summary_model_t::rowCount(const QModelIndex &parent) const {

  if (m_al) 
    return m_al->m_geom->n_atom_types();
  else 
    return 0;    
}

int qtype_summary_model_t::columnCount(const QModelIndex &parent) const {
  return 4;
}

QVariant qtype_summary_model_t::data(const QModelIndex &index, int role) const {

  if (!m_al) return QVariant();

  //return QVariant(42);

  if (role == Qt::DisplayRole) {

    int i = index.row();

    if (i>=0 && i<=m_al->m_geom->n_atom_types()) {
      
          auto ap_idx = ptable::number_by_symbol(atomic_name_to_symbol(m_al->m_geom->atom_of_type(i)));
	  //          switch (index.column()) {
	  //case 0 :
	  if (index.column()==0)
	    return QString::fromStdString(m_al->m_geom->atom_of_type(i));
	  //break;
	  else if (index.column()==1)
	    //case 1 :
	    return tr("%1").arg(m_al->m_geom->get_atom_count_by_type(i));
	    //break;
	  //case 2 :
	  else if (index.column()==2){
	    float pre_rad = 0.4f;
	    if (ap_idx)
	      pre_rad = ptable::get_inst()->arecs[*ap_idx - 1].m_radius;
	    auto itr = m_al->m_type_radius_override.find(i);
	    if (itr != m_al->m_type_radius_override.end())
	      pre_rad = itr -> second;	      
	    return QVariant(pre_rad);
	    // break;
	  }
	  else if  (index.column()==3){
	    //case 3 :
	      vector3<float> bc(0.0, 0.0, 1.0);
	      if (ap_idx) {bc = ptable::get_inst()->arecs[*ap_idx-1].m_color_jmol;}

	      QColor color_bck;

	      auto it = m_al->m_type_color_override.find(i);
	      if (it != m_al->m_type_color_override.end())
		color_bck.setRgbF(it->second[0], it->second[1], it->second[2]);
	      else color_bck.setRgbF(bc[0], bc[1], bc[2]);
              return QVariant(color_bck);
              //break;
	  }
    }

  }

  /*
  if(index.column() == 3 && role == Qt::CheckStateRole) {

      auto brec = m_al->m_tws_tr->m_bonding_table.m_dist.begin();
      std::advance(brec, index.row());
      if (brec->second.m_enabled) return Qt::Checked;
      else return Qt::Unchecked;

    }
  */
  if (role == Qt::TextAlignmentRole) {
      if (index.column() >= 0 && index.column() <= 3) return Qt::AlignCenter;
    }

  return QVariant();

}

QVariant qtype_summary_model_t::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const {

  if (role == Qt::DisplayRole) {

      if (orientation == Qt::Horizontal) {

          switch (section) {
            case 0:
              return tr("Name");
              break;
            case 1:
              return tr("Count");
              break;
            case 2:
              return tr("Radius");
              break;
            case 3:
              return tr("Color");
              break;
            default:
              return QString("");
            }

        }

    }

  return QVariant();

}

Qt::ItemFlags qtype_summary_model_t::flags(const QModelIndex &index) const {

  Qt::ItemFlags flags = Qt::ItemFlag::ItemIsEnabled;

  if (index.column() == 0 || index.column() == 1) {
      flags = flags | Qt::ItemFlag::ItemIsSelectable;
    }

  if (index.column() == 2) {
      flags = flags | Qt::ItemFlag::ItemIsEditable;
    }

  if (index.column() == 3) {
      flags = flags | Qt::ItemFlag::ItemIsEditable ;
    }

  return flags;

}

bool qtype_summary_model_t::setData(const QModelIndex &index, const QVariant &value, int role) {

  app_state_t* astate = app_state_t::get_inst();

  if (!m_al) return false;

  if (index.column() == 2 && role == Qt::EditRole) {

    int i = index.row();
    auto itr = m_al->m_type_radius_override.find(i);
    if (itr != m_al->m_type_radius_override.end()) m_al->m_type_radius_override.erase(itr);
    auto new_r = std::make_pair(size_t(i),value.toFloat());
    m_al->m_type_radius_override.insert(new_r);
    //m_al->update_from_ws_item();
    astate->make_viewport_dirty();
        
  }
  
  if (index.column() == 2 && role == Qt::EditRole) {

    auto clr = value.value<QColor>();
    if (clr.isValid()) {
      int i = index.row();
      auto it = m_al->m_type_color_override.find(i);
      if (it != m_al->m_type_color_override.end()) m_al->m_type_color_override.erase(it);
      auto new_c = std::make_pair(size_t(i),
				  vector3<float>(clr.redF(), clr.greenF(), clr.blueF()));
      m_al->m_type_color_override.insert(new_c);
      //m_al->update_from_ws_item();
      astate->make_viewport_dirty();
      
    }
  }

  return QAbstractTableModel::setData(index, value, role);

}

void qtype_summary_model_t::bind(geom_view_t *_al) {
  m_al = _al;
}

void qtype_summary_model_t::unbind() {
  //
  m_al = nullptr;
}
