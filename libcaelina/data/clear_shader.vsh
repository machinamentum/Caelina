.alias plz_no_crash c95 as (0.0, 4.0, 1.0, 8.0)

.alias outpos  o0      as position
.alias outcol  o1      as color
.alias outtex0 o2.xyzw as texcoord0

.alias projection  c0 - c3
.alias clear_color c5
.alias clear_depth c6

.alias vertex      v0
.alias v_texcoord  v1
.alias v_color     v2
.alias v_normal    v3

main:
    // temp.pos = mdlView * in.pos
    mov r1, vertex
    mov r1.z, clear_depth.z
    dp4 r0.x, projection[0], r1
    dp4 r0.y, projection[1], r1
    dp4 r0.z, projection[2], r1
    dp4 r0.w, projection[3], r1
    mov outpos, r0
    // result.texcoord = in.texcoord
    mov outtex0, v_texcoord
    // result.color = in.color
    mov outcol, clear_color
    nop
    end
endmain:
