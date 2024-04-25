import pyqpp as pq

def read_cell(l, line, cell, fields, l_cell):
    if l < l_cell : return
    fields = fields + [float(s) for s in line.split()]
    if len(fields) >= 9:
        k=0
        for i in [0,1,2]:
            for j in [0,1,2]:            
                cell[i,j] = fields[k]
                k += 1

def read_xgeom(filename, field_names, field_types, l_geom, read_cell, l_cell):

    cell = pq.periodic_cell_f(3 if read_cell else 0)
    for i in range(len(field_types)):
        if (field_types[i] =='float'):
            field_types[i] = 'real'
    header = list(zip(field_names, field_types))    
    geom = pq.xgeometry_f(cell, header)

    f = open(filename)
    l = 0
    ready_fields = []
    
    for line in f:
        l += 1
        if read_cell and len(ready_fields) < 9 :
            read_cell(l, line, geom.cell, ready_fields, l_cell)
        if l >= l_geom:
            fields = []
            ss = line.split()
            for i in range(geom.nfields() - len(ss)):
                ss.append('')
            for i in range(len(ss)):
                if field_types[i] == "int":
                    f = int(ss[i])
                elif field_types[i] == "real":
                    f = float(ss[i])
                elif field_types[i] == "string":
                    f = ss[i]
                elif field_types[i] == "bool":
                    f = ss[i].lower() in ['true', 'yes', 'on', 'y']
                fields.append(f)
            geom.add(fields)

    geom.name = filename
    geom.build_types()
    return geom

