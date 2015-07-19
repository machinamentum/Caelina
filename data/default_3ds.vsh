.alias plz_no_crash c95 as (0.0, 4.0, 1.0, 8.0)

.alias outpos  o0      as position
.alias outcol  o1      as color
.alias outtex0 o2.xyzw as texcoord0

.alias projection c0
.alias modelview  c4

.alias vertex      v0
.alias v_texcoord  v1
.alias v_color     v2
.alias v_normal    v3

main:
    // temp.pos = mdlView * in.pos
    dp4 r0.x, modelview[0], vertex
    dp4 r0.y, modelview[1], vertex
    dp4 r0.z, modelview[2], vertex
    dp4 r0.w, modelview[3], vertex
    // result.pos = prjMtx * temp.pos
    dp4 outpos.x, projection[0], r0
    dp4 outpos.y, projection[1], r0
    dp4 outpos.z, projection[2], r0
    dp4 outpos.w, projection[3], r0
    // result.texcoord = in.texcoord
    mov outtex0, v_texcoord
    // result.color = in.color
    mov outcol, v_color
    nop
    end
endmain:
