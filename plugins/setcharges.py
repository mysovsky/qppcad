def setcharges(geom, species, charges):

    found = False
    for q in range(geom.nfields()):
        if geom.field_name(q) == "charge":
            found = True
            break

    if not found:
        raise TypeError("Target geometry has no field for charges")
    
    for i in range(geom.nat()):
        try:
            j = species.index(geom.atom[i])
            geom.field[q,i] = charges[j]
        except ValueError:
            pass
        
