import pyqpp

def multmom(geom, Lmax):

    found = False
    for iq in range(geom.nfields()):
        if geom.field_name(iq) == "charge":
            found = True
            break

    if not found:
        raise TypeError("Target geometry has no charges")

    C = 0e0
    D = pyqpp.vector3f(0e0)
    Q = pyqpp.matrix3f(0e0)

    for i in range(geom.nat()):
        q = geom.field[iq,i]
        r = geom.pos(i)
        C += q
        D += q*r
        for j in [0,1,2]:
            for k in [0,1,2]:
                Q[j,k] += q*r[j]*r[k] - ( q*r.norm()**2/3 if j==k else 0e0)

    s = "Total charge : " + str(C) + "\nDipole moment : " +\
        str(D) + "\nQuadruple moment :\n" + str(Q)
    return s
